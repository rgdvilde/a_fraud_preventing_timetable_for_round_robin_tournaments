const fs = require('fs');
const path = require('path');

// Function to combine rows with the same round and game using FC as weights
function combineRowsByRoundAndGame(data, headers) {
    const combined = {};
    
    data.forEach((row, index) => {
        if (row.length < headers.length) {
            return; // Skip incomplete rows
        }
        
        const round = parseInt(row[0]);
        const game = parseInt(row[1]);
        const fc = parseFloat(row[19]); // FC column (index 19)
        const pS = parseFloat(row[20]); // Eg[p] column (index 20)
        const pSH = parseFloat(row[21]); // Eg[p | e0] column (index 21) - Home win
        const pSA = parseFloat(row[22]); // Eg[p | e1] column (index 22) - Away win
        const pSD = parseFloat(row[23]); // Eg[p | e2] column (index 23) - Draw
        
        const key = `${round}_${game}`;
        
        if (!combined[key]) {
            combined[key] = {
                round: round,
                game: game,
                totalWeight: 0,
                weightedPS: 0,
                weightedPSH: 0,
                weightedPSA: 0,
                weightedPSD: 0,
                rows: []
            };
        }
        
        // Add weighted contributions
        combined[key].totalWeight += fc;
        combined[key].weightedPS += pS * fc;
        combined[key].weightedPSH += pSH * fc;
        combined[key].weightedPSA += pSA * fc;
        combined[key].weightedPSD += pSD * fc;
        combined[key].rows.push({
            fc: fc,
            pS: pS,
            pSH: pSH,
            pSA: pSA,
            pSD: pSD
        });
    });
    
    // Convert to normalized data
    const normalizedData = [];
    Object.values(combined).forEach(group => {
        if (group.totalWeight > 0) {
            normalizedData.push([
                group.round,
                group.game,
                group.weightedPS / group.totalWeight,  // Normalized Eg[p]
                group.weightedPSH / group.totalWeight, // Normalized Eg[p | e0] - Home win
                group.weightedPSA / group.totalWeight, // Normalized Eg[p | e1] - Away win
                group.weightedPSD / group.totalWeight  // Normalized Eg[p | e2] - Draw
            ]);
        }
    });
    
    return normalizedData;
}

// Function to process data for a specific round and game
function processData(round, game, csvData, bucketSize = 0.05) {
    // Get the original CSV data for this round and game
    const filteredData = csvData.filter(row => {
        return parseInt(row[0]) === round && parseInt(row[1]) === game;
    });

    // Calculate total FC for this round/game pair
    const totalFC = filteredData.reduce((sum, row) => sum + parseFloat(row[19]), 0);

    // Create raw data arrays for cumulative charts
    const rawDataH = filteredData.map(row => ({
        x: parseFloat(row[21]), // Eg[p | e0] - x-axis (Home win)
        y: parseFloat(row[19]) / totalFC  // FC relative to total - y-axis
    }));

    const rawDataA = filteredData.map(row => ({
        x: parseFloat(row[22]), // Eg[p | e1] - x-axis (Away win)
        y: parseFloat(row[19]) / totalFC  // FC relative to total - y-axis
    }));

    const rawDataD = filteredData.map(row => ({
        x: parseFloat(row[23]), // Eg[p | e2] - x-axis (Draw)
        y: parseFloat(row[19]) / totalFC  // FC relative to total - y-axis
    }));

    // Create raw data for cumulative chart using Eg[p] (P(S) column)
    const rawDataPS = filteredData.map(row => ({
        x: parseFloat(row[20]), // Eg[p] - x-axis (P(S) column)
        y: parseFloat(row[19]) / totalFC  // FC relative to total - y-axis
    }));

    // Calculate FC(x) - ratio of FC values where P(S) >= 0 (default threshold)
    // FC(x) = sum(FC where P(S) >= x) / sum(FC)
    const fcAboveZero = filteredData
        .filter(row => parseFloat(row[20]) >= 0)
        .reduce((sum, row) => sum + parseFloat(row[19]), 0);
    const totalYPSAboveZero = totalFC > 0 ? fcAboveZero / totalFC : 0;

    // Calculate FS(x) directly using original FC values (test.js approach)
    // FS(x) = sum(FC * max(0, P(S) - x)) / sum(FC) where x = 0 (default)
    let fsXWeightedSum = 0;
    filteredData.forEach(row => {
        const pS = parseFloat(row[20]); // P(S)
        const fc = parseFloat(row[19]); // FC
        
        if (pS > 0) { // Only consider points where P(S) > 0
            const pClamped = Math.min(1, pS); // Keep it in [0,1]
            fsXWeightedSum += fc * pClamped;
        }
    });
    const fsX = totalFC > 0 ? fsXWeightedSum / totalFC : 0;

    const bucketsH = {};
    const bucketsA = {};
    const bucketsD = {};
    
    // Calculate weighted averages from raw data
    let totalWeightH = 0;
    let weightedSumH = 0;
    let totalWeightA = 0;
    let weightedSumA = 0;
    let totalWeightD = 0;
    let weightedSumD = 0;

    filteredData.forEach(row => {
        const xValueH = parseFloat(row[21]); // P(S|H) - x-axis
        const xValueA = parseFloat(row[22]); // P(S|A) - x-axis
        const xValueD = parseFloat(row[23]); // P(S|D) - x-axis
        const yValue = parseFloat(row[19]) / totalFC; // FC relative to total - y-axis

        // Calculate weighted sums for averages (using FC as weight)
        const fc = parseFloat(row[19]);
        weightedSumH += xValueH * fc;
        totalWeightH += fc;
        weightedSumA += xValueA * fc;
        totalWeightA += fc;
        weightedSumD += xValueD * fc;
        totalWeightD += fc;

        // Process buckets for bar chart - P(S|H)
        const bucketIndexH = Math.floor(xValueH / bucketSize);
        const bucketKeyH = (bucketIndexH * bucketSize).toFixed(2);
        if (!bucketsH[bucketKeyH]) {
            bucketsH[bucketKeyH] = {
                count: 0,
                totalY: 0
            };
        }
        bucketsH[bucketKeyH].count++;
        bucketsH[bucketKeyH].totalY += yValue;

        // Process buckets for bar chart - P(S|A)
        const bucketIndexA = Math.floor(xValueA / bucketSize);
        const bucketKeyA = (bucketIndexA * bucketSize).toFixed(2);
        if (!bucketsA[bucketKeyA]) {
            bucketsA[bucketKeyA] = {
                count: 0,
                totalY: 0
            };
        }
        bucketsA[bucketKeyA].count++;
        bucketsA[bucketKeyA].totalY += yValue;

        // Process buckets for bar chart - P(S|D)
        const bucketIndexD = Math.floor(xValueD / bucketSize);
        const bucketKeyD = (bucketIndexD * bucketSize).toFixed(2);
        if (!bucketsD[bucketKeyD]) {
            bucketsD[bucketKeyD] = {
                count: 0,
                totalY: 0
            };
        }
        bucketsD[bucketKeyD].count++;
        bucketsD[bucketKeyD].totalY += yValue;
    });

    // Calculate averages
    const avgH = totalWeightH > 0 ? weightedSumH / totalWeightH : 0;
    const avgA = totalWeightA > 0 ? weightedSumA / totalWeightA : 0;
    const avgD = totalWeightD > 0 ? weightedSumD / totalWeightD : 0;
    
    // Calculate P(S) average (FC-weighted mean of P(S) values)
    let totalWeightPS = 0;
    let weightedSumPS = 0;
    filteredData.forEach(row => {
        const pS = parseFloat(row[20]); // Eg[p] - P(S) column
        const fc = parseFloat(row[19]);
        weightedSumPS += pS * fc;
        totalWeightPS += fc;
    });
    const avgPS = totalWeightPS > 0 ? weightedSumPS / totalWeightPS : 0;

    // Calculate y values for each bucket for bar chart
    const bucketDataH = Object.entries(bucketsH).map(([x, data]) => ({
        x: parseFloat(x),
        y: data.totalY
    }));

    const bucketDataA = Object.entries(bucketsA).map(([x, data]) => ({
        x: parseFloat(x),
        y: data.totalY
    }));

    const bucketDataD = Object.entries(bucketsD).map(([x, data]) => ({
        x: parseFloat(x),
        y: data.totalY
    }));

    // Sort buckets by x value
    bucketDataH.sort((a, b) => a.x - b.x);
    bucketDataA.sort((a, b) => a.x - b.x);
    bucketDataD.sort((a, b) => a.x - b.x);

    // Sort raw data by x value for cumulative chart
    rawDataH.sort((a, b) => b.x - a.x);
    rawDataA.sort((a, b) => b.x - a.x);
    rawDataD.sort((a, b) => b.x - a.x);
    rawDataPS.sort((a, b) => b.x - a.x);

    return { bucketDataH, bucketDataA, bucketDataD, rawDataH, rawDataA, rawDataD, rawDataPS, avgH, avgA, avgD, avgPS, totalYPSAboveZero, fsX };
}

// Function to create cumulative data from raw data points
function createCumulativeData(rawData) {
    let cumulativeSum = 0;
    return rawData.map(point => {
        cumulativeSum += point.y;
        return {
            x: point.x,
            y: cumulativeSum
        };
    });
}

// Function to create bar chart data with fixed x-axis from -1 to 1 (as {x, y} pairs)
function createFixedBarChartData(bucketDataH, bucketDataA, bucketDataD, avgH, avgA, avgD, avgPS, interval = 0.05) {
    const xValues = [];
    for (let x = -1; x <= 1 + 1e-8; x += interval) {
        xValues.push(Number(x.toFixed(2)));
    }
    return {
        dataH: xValues.map(x => {
            const bucket = bucketDataH.find(d => Math.abs(d.x - x) < 1e-8);
            return { x, y: bucket ? bucket.y : 0 };
        }),
        dataA: xValues.map(x => {
            const bucket = bucketDataA.find(d => Math.abs(d.x - x) < 1e-8);
            return { x, y: bucket ? bucket.y : 0 };
        }),
        dataD: xValues.map(x => {
            const bucket = bucketDataD.find(d => Math.abs(d.x - x) < 1e-8);
            return { x, y: bucket ? bucket.y : 0 };
        }),
        avgH, avgA, avgD, avgPS
    };
}

// Function to create cumulative chart data with fixed x-axis from -1 to 1 (as {x, y} pairs)
function createFixedCumulativeChartData(rawDataH, rawDataA, rawDataD, rawDataPS, avgH, avgA, avgD, avgPS, interval = 0.1) {
    // --- TRUE INVERSE CDF FOR ALL LINES ---
    function makeInverseCDF(rawData) {
        const sorted = [...rawData].sort((a, b) => b.x - a.x);
        return sorted.map((point, idx) => {
            let y = 0;
            for (let i = 0; i <= idx; i++) {
                y += sorted[i].y;
            }
            return { x: point.x, y };
        });
    }
    
    // Function to calculate integral (area under curve) between 0 and 1
    // This calculates FS(x) = integral of survival curve from 0 to 1
    // Using the test.js approach: FS(x) = sum(FC * max(0, P(S) - x)) / sum(FC)
    function calculateIntegral(data) {
        if (data.length < 2) return 0;
        
        // Sort data by x value
        const sortedData = [...data].sort((a, b) => a.x - b.x);
        
        // Find points within [0, 1] range
        const pointsInRange = sortedData.filter(point => point.x >= 0 && point.x <= 1);
        
        if (pointsInRange.length < 2) return 0;
        
        // Calculate area using trapezoidal rule
        let integral = 0;
        for (let i = 1; i < pointsInRange.length; i++) {
            const x1 = pointsInRange[i-1].x;
            const y1 = pointsInRange[i-1].y;
            const x2 = pointsInRange[i].x;
            const y2 = pointsInRange[i].y;
            
            // Area of trapezoid: (base1 + base2) * height / 2
            const area = (y1 + y2) * (x2 - x1) / 2;
            integral += area;
        }
        
        return integral;
    }
    
    const dataH = makeInverseCDF(rawDataH);
    const dataA = makeInverseCDF(rawDataA);
    const dataD = makeInverseCDF(rawDataD);
    const dataPS = makeInverseCDF(rawDataPS);

    // Calculate integrals using the test.js approach
    // FS(x) = sum(FC * max(0, P(S) - x)) / sum(FC) where x = 0 (default)
    function calculateFSX(data, threshold = 0) {
        if (data.length === 0) return 0;
        
        // Since data contains normalized weights (FC/totalFC), we need to reconstruct the calculation
        // FS(x) = sum(FC * max(0, P(S) - x)) / sum(FC)
        // But we have FC/totalFC, so we need to multiply by totalFC to get back to FC values
        let totalFC = 0;
        let weightedSum = 0;
        
        data.forEach(point => {
            const p = point.x - threshold; // P(S) - x
            const normalizedFC = point.y; // FC/totalFC
            
            if (p > 0) { // Only consider points where P(S) > x
                const pClamped = Math.min(1, p); // Keep it in [0,1]
                totalFC += normalizedFC; // This will sum to 1 since weights are normalized
                weightedSum += normalizedFC * pClamped;
            }
        });
        
        // Since totalFC sums to 1 (normalized), we return the weighted sum directly
        return weightedSum;
    }
    
    // Calculate FS(x) values using the test.js approach
    // FS(x) = sum(FC * max(0, P(S) - x)) / sum(FC) where x = 0 (default)
    const integralH = calculateFSX(rawDataH);
    const integralA = calculateFSX(rawDataA);
    const integralD = calculateFSX(rawDataD);
    const integralPS = calculateFSX(rawDataPS);

    return {
        dataH,
        dataA,
        dataD,
        dataPS,
        avgH, avgA, avgD, avgPS,
        integralH, integralA, integralD, integralPS
    };
}

// Function to generate HTML template
function generateHTMLTemplate(title, chartData, chartType = 'bar') {
    const isBarChart = chartType === 'bar';
    const isCumulative = chartType === 'cumulative';
    return `<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>${title}</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/chartjs-plugin-annotation@1.4.0/dist/chartjs-plugin-annotation.min.js"></script>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background-color: #f5f5f5; }
        .container { max-width: 1200px; margin: 0 auto; background-color: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        h1 { color: #333; text-align: center; margin-bottom: 30px; }
        .chart-container { position: relative; height: 400px; margin-bottom: 30px; }
        .stats { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 15px; margin-top: 20px; }
        .stat-card { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; padding: 15px; border-radius: 8px; text-align: center; }
        .stat-card h3 { margin: 0 0 10px 0; font-size: 14px; opacity: 0.9; }
        .stat-card .value { font-size: 24px; font-weight: bold; }
        .controls { display: flex; justify-content: center; align-items: center; gap: 15px; margin-bottom: 20px; padding: 15px; background-color: #f8f9fa; border-radius: 8px; }
        .control-group { display: flex; align-items: center; gap: 8px; }
        .control-group label { font-weight: bold; color: #333; }
        .control-group input { padding: 8px 12px; border: 1px solid #ddd; border-radius: 4px; font-size: 14px; width: 100px; }
        .control-group button { padding: 8px 16px; background-color: #007bff; color: white; border: none; border-radius: 4px; cursor: pointer; font-size: 14px; }
        .control-group button:hover { background-color: #0056b3; }
    </style>
</head>
<body>
    <div class="container">
        <h1>${title}</h1>
        <div class="controls">
            <div class="control-group">
                <label for="xOffset">X-Axis Offset:</label>
                <input type="number" id="xOffset" value="0" step="0.01" placeholder="0">
                <button onclick="applyOffset()">Apply</button>
            </div>
        </div>
        <div class="chart-container">
            <canvas id="chart"></canvas>
        </div>
                    <div class="stats">
            ${isCumulative ? `
            <div class="stat-card" style="background: linear-gradient(135deg, #ff6b6b 0%, #ee5a24 100%);">
                <h3><strong>FS(x)</strong></h3>
                <div class="value" data-original="${chartData.integralPS || 0}" style="font-size: 28px; font-weight: bold;">${chartData.integralPS ? chartData.integralPS.toFixed(3) : 'N/A'}</div>
            </div>
            <div class="stat-card" style="background: linear-gradient(135deg, #28a745 0%, #20c997 100%);">
                <h3><strong>FC(x)</strong></h3>
                <div class="value" data-original="${chartData.totalYPSAboveZero || 0}" style="font-size: 28px; font-weight: bold;">${chartData.totalYPSAboveZero ? chartData.totalYPSAboveZero.toFixed(3) : 'N/A'}</div>
            </div>
            ` : ''}
            <div class="stat-card">
                <h3>Eg[p]</h3>
                <div class="value" data-original="${chartData.avgPS || 0}">${chartData.avgPS ? chartData.avgPS.toFixed(3) : 'N/A'}</div>
            </div>
            <div class="stat-card">
                <h3>Eg[p | e0]</h3>
                <div class="value" data-original="${chartData.avgH || 0}">${chartData.avgH ? chartData.avgH.toFixed(3) : 'N/A'}</div>
            </div>
            <div class="stat-card">
                <h3>Eg[p | e1]</h3>
                <div class="value" data-original="${chartData.avgA || 0}">${chartData.avgA ? chartData.avgA.toFixed(3) : 'N/A'}</div>
            </div>
            <div class="stat-card">
                <h3>Eg[p | e2]</h3>
                <div class="value" data-original="${chartData.avgD || 0}">${chartData.avgD ? chartData.avgD.toFixed(3) : 'N/A'}</div>
            </div>

        </div>
    </div>
    <script>
        let chart;
        let originalData = {
            dataH: ${JSON.stringify(chartData.dataH)},
            dataA: ${JSON.stringify(chartData.dataA)},
            dataD: ${JSON.stringify(chartData.dataD)}
            ${isBarChart ? '' : ', dataPS: ' + JSON.stringify(chartData.dataPS)}
        };
        
        // Store original FC value and raw data for offset calculations
        const originalTotalYPSAboveZero = ${chartData.totalYPSAboveZero || 0};
        const originalRawDataPS = ${JSON.stringify(chartData.originalRawDataPS || chartData.dataPS)};
        
        // Store the original FS(x) value for offset calculations
        const originalFSX = ${chartData.integralPS || 0};
        
        // Function to calculate FS(x) for shifted data using test.js approach
        // This should match exactly the overview calculation method
        function calculateFSXForOffset(data, offset) {
            if (data.length === 0) return 0;
            
            // Apply offset to data points
            const shiftedData = data.map(point => ({ x: point.x - offset, y: point.y }));
            
            // Calculate FS(x) = sum(FC * max(0, P(S) - x)) / sum(FC) where x = offset
            // Since the data is normalized (FC/totalFC), we return the weighted sum directly
            // This matches the overview calculation method exactly
            let weightedSum = 0;
            
            shiftedData.forEach(point => {
                const p = point.x; // P(S) - offset (already shifted)
                const normalizedFC = point.y; // FC/totalFC (normalized weight)
                
                if (p > 0) { // Only consider points where P(S) - offset > 0
                    const pClamped = Math.min(1, p); // Keep it in [0,1]
                    weightedSum += normalizedFC * pClamped;
                }
            });
            
            // Return the weighted sum directly (same as overview)
            return weightedSum;
        }
        
        // Function to calculate FC(x) - ratio of FC values where P(S) >= offset
        function calculateFCForOffset(data, offset) {
            if (data.length === 0) return 0;
            
            // Apply offset to data points
            const shiftedData = data.map(point => ({ x: point.x - offset, y: point.y }));
            
            // Calculate ratio of FC values where P(S) >= 0 after offset
            // y values are already normalized FC values (FC/totalFC)
            // FC(x) = sum(FC where P(S) >= x) / sum(FC)
            const totalYAboveZero = shiftedData
                .filter(point => point.x >= 0)
                .reduce((sum, point) => sum + point.y, 0);
            
            return totalYAboveZero;
        }
        
        function applyOffset() {
            const offset = parseFloat(document.getElementById('xOffset').value) || 0;
            
            // Apply offset to all datasets
            const offsetDataH = originalData.dataH.map(point => ({ x: point.x - offset, y: point.y }));
            const offsetDataA = originalData.dataA.map(point => ({ x: point.x - offset, y: point.y }));
            const offsetDataD = originalData.dataD.map(point => ({ x: point.x - offset, y: point.y }));
            
            // Update chart data
            chart.data.datasets[0].data = offsetDataH;
            chart.data.datasets[1].data = offsetDataA;
            chart.data.datasets[2].data = offsetDataD;
            
            ${isBarChart ? '' : 'const offsetDataPS = originalData.dataPS.map(point => ({ x: point.x - offset, y: point.y })); chart.data.datasets[3].data = offsetDataPS;'}
            
            // Keep fixed x-axis ranges regardless of offset
            chart.options.scales.x.min = ${isBarChart ? '-1' : '0'};
            chart.options.scales.x.max = '1';
            
            // Update displayed averages with offset
            const valueElements = document.querySelectorAll('.stat-card .value');
            valueElements.forEach(element => {
                const originalValue = parseFloat(element.getAttribute('data-original')) || 0;
                const h3Text = element.parentElement.querySelector('h3').textContent;
                
                // Handle FC(x) values - calculate new FC value with offset
                if (h3Text.includes('FC(x)')) {
                    const newFC = calculateFCForOffset(originalRawDataPS, offset);
                    element.textContent = newFC.toFixed(3);
                } else if (h3Text.includes('FS(x)')) {
                    // Handle FS(x) values - calculate new FS(x) value with offset
                    const newFSX = calculateFSXForOffset(originalRawDataPS, offset);
                    element.textContent = newFSX.toFixed(3);
                } else {
                    const adjustedValue = originalValue - offset;
                    element.textContent = adjustedValue.toFixed(3);
                }
            });
            
            // Update integral values with offset and recalculate colors
            const integralElements = document.querySelectorAll('.stat-card .value[data-original]');
            const integralValues = [];
            
            integralElements.forEach(element => {
                const h3Text = element.parentElement.querySelector('h3').textContent;
                
                if (h3Text.includes('Integral')) {
                    let newIntegral = 0;
                    
                    if (h3Text.includes('Eg[p | e0]')) {
                        newIntegral = calculateFSXForOffset(originalData.dataH, offset);
                    } else if (h3Text.includes('Eg[p | e1]')) {
                        newIntegral = calculateFSXForOffset(originalData.dataA, offset);
                    } else if (h3Text.includes('Eg[p | e2]')) {
                        newIntegral = calculateFSXForOffset(originalData.dataD, offset);
                    } else if (h3Text.includes('Eg[p]')) {
                        newIntegral = calculateFSXForOffset(originalData.dataPS, offset);
                    }
                    
                    integralValues.push(newIntegral);
                    element.textContent = newIntegral.toFixed(3);
                }
            });
            
            // Update gradient colors based on new integral values
            if (integralValues.length > 0) {
                const minIntegral = Math.min(...integralValues);
                const maxIntegral = Math.max(...integralValues);
                const integralRange = maxIntegral - minIntegral;
                
                integralElements.forEach((element, index) => {
                    if (index < integralValues.length) {
                        const newIntegral = integralValues[index];
                        const statCard = element.closest('.stat-card');
                        
                        // Calculate new color based on adjusted integral value
                        let integralColor = '#f0f0f0';
                        if (!isNaN(newIntegral) && integralRange > 0) {
                            const normalized = (newIntegral - minIntegral) / integralRange;
                            const red = Math.round(255 * normalized);
                            const green = Math.round(255 * (1 - normalized));
                            integralColor = 'rgb(' + red + ', ' + green + ', 0)';
                        }
                        
                        // Update card background color
                        statCard.style.background = 'linear-gradient(135deg, ' + integralColor + '15 0%, ' + integralColor + '25 100%)';
                    }
                });
            }
            
            chart.update();
        }

        const ctx = document.getElementById('chart').getContext('2d');
        chart = new Chart(ctx, {
            type: '${isBarChart ? 'bar' : 'line'}',
            data: {
                datasets: [
                    {
                        label: 'Eg[p | e0]',
                        data: ${JSON.stringify(chartData.dataH)},
                        backgroundColor: 'rgba(54, 162, 235, 0.8)',
                        borderColor: 'rgba(54, 162, 235, 1)',
                        borderWidth: 1,
                        ${isBarChart ? '' : 'type: \'line\', showLine: true, pointRadius: 0, fill: false,'}
                    },
                    {
                        label: 'Eg[p | e1]',
                        data: ${JSON.stringify(chartData.dataA)},
                        backgroundColor: 'rgba(255, 99, 132, 0.8)',
                        borderColor: 'rgba(255, 99, 132, 1)',
                        borderWidth: 1,
                        ${isBarChart ? '' : 'type: \'line\', showLine: true, pointRadius: 0, fill: false,'}
                    },
                    {
                        label: 'Eg[p | e2]',
                        data: ${JSON.stringify(chartData.dataD)},
                        backgroundColor: 'rgba(75, 192, 192, 0.8)',
                        borderColor: 'rgba(75, 192, 192, 1)',
                        borderWidth: 1,
                        ${isBarChart ? '' : 'type: \'line\', showLine: true, pointRadius: 0, fill: false,'}
                    }
                    ${isBarChart ? '' : ',\n                    {\n                        label: \'Eg[p]\',\n                        data: ' + JSON.stringify(chartData.dataPS) + ',\n                        borderColor: \'rgba(0, 0, 0, 1)\',\n                        backgroundColor: \'rgba(0, 0, 0, 0.1)\',\n                        borderWidth: 3,\n                        type: \'line\', showLine: true, pointRadius: 0, fill: false\n                    }'}
                ]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: {
                    title: {
                        display: true,
                        text: '${isBarChart ? "Probability Distribution by Buckets" : "Cumulative Probability Distribution"}',
                        font: { size: 16 }
                    },
                    legend: { display: true, position: 'top' },
                    annotation: {
                        annotations: {
                            centerLine: {
                                type: 'line',
                                xMin: 0,
                                xMax: 0,
                                borderColor: 'rgba(100,100,100,0.7)',
                                borderWidth: 2,
                                borderDash: [4, 4],
                                label: {
                                    display: false
                                },
                                z: 10
                            }
                        }
                    }
                },
                scales: {
                    x: {
                        type: 'linear',
                        min: -1,
                        max: 1,
                        title: { display: true, text: 'Eg[p|X]' },
                        grid: {
                            drawOnChartArea: true,
                            drawTicks: true
                        },
                        axis: 'x',
                        position: 'bottom',
                        offset: false,
                        ticks: {
                            // No special styling for x=0
                        }
                    },
                    y: {
                        title: { display: true, text: 'Probability' },
                        axis: 'y',
                        position: 'left',
                        offset: false
                    }
                }
            }
        });

        // Add enter key support for the input
        document.getElementById('xOffset').addEventListener('keypress', function(e) {
            if (e.key === 'Enter') {
                applyOffset();
            }
        });
    </script>
</body>
</html>`;
}

// Function to create HTML visualization for a specific round and game
function createHTMLVisualization(round, game, bucketDataH, bucketDataA, bucketDataD, rawDataH, rawDataA, rawDataD, rawDataPS, avgH, avgA, avgD, avgPS, totalYPSAboveZero, fsX) {
    const dir = `./visualizations`;
    if (!fs.existsSync(dir)){
        fs.mkdirSync(dir, { recursive: true });
    }

    // Use fixed x-axis for bar and cumulative charts
    const barChartData = createFixedBarChartData(bucketDataH, bucketDataA, bucketDataD, avgH, avgA, avgD, avgPS, 0.05);
    const cumulativeChartData = createFixedCumulativeChartData(rawDataH, rawDataA, rawDataD, rawDataPS, avgH, avgA, avgD, avgPS, 0.1);

    // Override the integralPS with the pre-calculated fsX value
    cumulativeChartData.integralPS = fsX;

    // Add totalYPSAboveZero and original raw data to both chart data objects
    barChartData.totalYPSAboveZero = totalYPSAboveZero;
    cumulativeChartData.totalYPSAboveZero = totalYPSAboveZero;
    barChartData.originalRawDataPS = rawDataPS;
    cumulativeChartData.originalRawDataPS = rawDataPS;

    // Generate HTML files
    const barChartHTML = generateHTMLTemplate(`Round ${round}, Game ${game} - Bar Chart`, barChartData, 'bar');
    const cumulativeHTML = generateHTMLTemplate(`Round ${round}, Game ${game} - Cumulative Chart`, cumulativeChartData, 'cumulative');

    fs.writeFileSync(`${dir}/round_${round}_game_${game}_bar.html`, barChartHTML);
    fs.writeFileSync(`${dir}/round_${round}_game_${game}_cumulative.html`, cumulativeHTML);

    return { barChartData, cumulativeChartData };
}

// Function to create combined overview HTML
function createCombinedOverview(allGraphs, sortedRounds, sortedGames) {
    const dir = `./visualizations`;
    if (!fs.existsSync(dir)){
        fs.mkdirSync(dir, { recursive: true });
    }

    // Calculate min and max P(S) integral values for gradient
    const integralValues = allGraphs.map(graph => {
        const cumulativeData = createFixedCumulativeChartData(
            graph.rawDataH,
            graph.rawDataA,
            graph.rawDataD,
            graph.rawDataPS,
            graph.avgH,
            graph.avgA,
            graph.avgD,
            graph.avgPS,
            0.1
        );
        return cumulativeData.integralPS;
    }).filter(val => !isNaN(val));
    
    const minIntegral = Math.min(...integralValues);
    const maxIntegral = Math.max(...integralValues);
    const integralRange = maxIntegral - minIntegral;
    const avgFSInitial = integralValues.length > 0 ? (integralValues.reduce((a,b)=>a+b,0) / integralValues.length) : 0;
    const fcValuesInitial = allGraphs.map(g => g.totalYPSAboveZero).filter(v => !isNaN(v));
    const avgFCInitial = fcValuesInitial.length > 0 ? (fcValuesInitial.reduce((a,b)=>a+b,0) / fcValuesInitial.length) : 0;

    // Function to get color based on P(S) integral value
    function getColorForIntegral(integralValue) {
        if (isNaN(integralValue) || integralRange === 0) return '#f0f0f0';
        const normalized = (integralValue - minIntegral) / integralRange;
        // Green (low) to Red (high) gradient
        const red = Math.round(255 * normalized);
        const green = Math.round(255 * (1 - normalized));
        return `rgb(${red}, ${green}, 0)`;
    }

    // Create overview HTML with links to all charts
    let overviewHTML = `<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Expected Value Visualization</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 20px;
            background-color: #f5f5f5;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
            background-color: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }
        h1 {
            color: #333;
            text-align: center;
            margin-bottom: 30px;
        }
        .toggle-container {
            display: flex;
            justify-content: center;
            margin-bottom: 20px;
        }
        .chart-type-select {
            padding: 8px 16px;
            font-size: 14px;
            border: 1px solid #ddd;
            border-radius: 6px;
            background: white;
            cursor: pointer;
        }
        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
        }
        .legend {
            display: flex;
            justify-content: center;
            margin-bottom: 20px;
            padding: 10px;
            background-color: #f8f9fa;
            border-radius: 6px;
        }
        .legend-item {
            display: flex;
            align-items: center;
            gap: 10px;
            font-size: 14px;
            color: #666;
        }
        .legend-color {
            width: 100px;
            height: 20px;
            border-radius: 4px;
            border: 1px solid #ddd;
        }
        .summary {
            display: flex;
            justify-content: space-around;
            margin-bottom: 20px;
            padding: 15px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            border-radius: 8px;
        }
        .summary-item {
            text-align: center;
            font-size: 14px;
        }
        .chart-card {
            border: 1px solid #ddd;
            border-radius: 8px;
            padding: 15px;
            background-color: #fafafa;
            transition: transform 0.2s, box-shadow 0.2s;
            cursor: pointer;
            position: relative;
        }
        .chart-card:hover {
            transform: translateY(-2px);
            box-shadow: 0 4px 12px rgba(0,0,0,0.15);
        }
        .chart-card:active {
            transform: translateY(0px);
            box-shadow: 0 2px 6px rgba(0,0,0,0.1);
        }
        .chart-card h3 {
            margin: 0 0 10px 0;
            color: #333;
        }

        .stats {
            margin-top: 10px;
            font-size: 12px;
            color: #666;
        }
        .stat-row {
            display: flex;
            justify-content: space-between;
            margin-bottom: 4px;
        }
        .stat-row:last-child {
            margin-bottom: 0;
        }
        .stat-row div {
            flex: 1;
            padding: 2px 4px;
        }
        .ps-value {
            color: #333;
            font-weight: bold;
        }
        .click-hint {
            text-align: center;
            font-size: 11px;
            color: #888;
            margin-top: 8px;
            opacity: 0.7;
            font-style: italic;
        }
        .overview-controls {
            display: flex;
            justify-content: center;
            align-items: center;
            gap: 15px;
            margin-bottom: 20px;
            padding: 15px;
            background-color: #f8f9fa;
            border-radius: 8px;
        }
        .overview-control-group {
            display: flex;
            align-items: center;
            gap: 8px;
        }
        .overview-control-group label {
            font-weight: bold;
            color: #333;
        }
        .overview-control-group input {
            padding: 8px 12px;
            border: 1px solid #ddd;
            border-radius: 4px;
            font-size: 14px;
            width: 100px;
        }
        .overview-control-group button {
            padding: 8px 16px;
            background-color: #007bff;
            color: white;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 14px;
        }
        .overview-control-group button:hover {
            background-color: #0056b3;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Expected Value Visualization</h1>
        <div class="legend">
            <div class="legend-item">
                <div class="legend-color" style="background: linear-gradient(90deg, #00ff00, #ff0000);"></div>
                <span>FS(x) Values: Green (Low) → Red (High)</span>
            </div>
        </div>
        <div class="summary" id="summary">
            <div class="summary-item">
                <strong><span id="highestLabel">Highest FS(x):</span></strong> <span id="highestIntegral">${maxIntegral.toFixed(3)}</span>
                <span id="highestIntegralLocation">${(() => {
                    const highest = allGraphs.find(g => {
                        const cumulativeData = createFixedCumulativeChartData(
                            g.rawDataH, g.rawDataA, g.rawDataD, g.rawDataPS,
                            g.avgH, g.avgA, g.avgD, g.avgPS, 0.1
                        );
                        return Math.abs(cumulativeData.integralPS - maxIntegral) < 0.001;
                    });
                    return highest ? `(Round ${highest.round}, Game ${highest.game})` : '';
                })()}</span>
            </div>
            <div class="summary-item">
                <strong><span id="lowestLabel">Lowest FS(x):</span></strong> <span id="lowestIntegral">${minIntegral.toFixed(3)}</span>
                <span id="lowestIntegralLocation">${(() => {
                    const lowest = allGraphs.find(g => {
                        const cumulativeData = createFixedCumulativeChartData(
                            g.rawDataH, g.rawDataA, g.rawDataD, g.rawDataPS,
                            g.avgH, g.avgA, g.avgD, g.avgPS, 0.1
                        );
                        return Math.abs(cumulativeData.integralPS - minIntegral) < 0.001;
                    });
                    return lowest ? `(Round ${lowest.round}, Game ${lowest.game})` : '';
                })()}</span>
            </div>
            <div class="summary-item">
                <strong>Average FS(x):</strong> <span id="avgFS">${avgFSInitial.toFixed(3)}</span>
            </div>
            <div class="summary-item">
                <strong>Average FC(x):</strong> <span id="avgFC">${avgFCInitial.toFixed(3)}</span>
            </div>
        </div>
        <div class="overview-controls">
            <div class="overview-control-group">
                <label for="overviewXOffset">X-Axis Offset:</label>
                <input type="number" id="overviewXOffset" value="0" step="0.01" placeholder="0">
                <button onclick="applyOverviewOffset()">Apply</button>
            </div>
            <div class="overview-control-group">
                <label for="togglePreview">Chart Type:</label>
                <select id="togglePreview" class="chart-type-select">
                    <option value="cumulative">Cumulative Preview</option>
                    <option value="bar">Bar Chart Preview</option>
                </select>
            </div>
            <div class="overview-control-group">
                <label for="colorMetricSelect">Color metric:</label>
                <select id="colorMetricSelect" class="chart-type-select">
                    <option value="FS">FS</option>
                    <option value="FC">FC</option>
                </select>
            </div>
        </div>
        <div class="grid">`;

    let chartScripts = '';
    let chartDataMap = {};
    let chartIndex = 0;
    sortedRounds.forEach(round => {
        sortedGames.forEach(game => {
            const graph = allGraphs.find(g => g.round === round && g.game === game);
            if (graph) {
                const chartId = `preview_chart_${round}_${game}`;
                // Precompute both chart data
                const cumulativeData = createFixedCumulativeChartData(
                    graph.rawDataH,
                    graph.rawDataA,
                    graph.rawDataD,
                    graph.rawDataPS,
                    graph.avgH,
                    graph.avgA,
                    graph.avgD,
                    graph.avgPS,
                    0.1
                );
                const barData = createFixedBarChartData(
                    graph.bucketDataH,
                    graph.bucketDataA,
                    graph.bucketDataD,
                    graph.avgH,
                    graph.avgA,
                    graph.avgD,
                    graph.avgPS,
                    0.05
                );
                // Store both for JS access
                chartDataMap[chartId] = {
                    cumulative: {
                        type: 'line',
                        data: {
                            datasets: [
                                {
                                    label: 'P(S)',
                                    data: cumulativeData.dataPS,
                                    borderColor: 'rgba(0, 0, 0, 1)',
                                    backgroundColor: 'rgba(0, 0, 0, 0.1)',
                                    borderWidth: 3,
                                    type: 'line',
                                    showLine: true,
                                    pointRadius: 0,
                                    fill: false
                                }
                            ]
                        }
                    },
                    // Store original raw data for FC(x) calculations
                    rawDataPS: graph.rawDataPS,
                    bar: {
                        type: 'bar',
                        data: {
                            datasets: [
                                {
                                    label: 'P(S|H)',
                                    data: barData.dataH,
                                    backgroundColor: 'rgba(54, 162, 235, 0.8)',
                                    borderColor: 'rgba(54, 162, 235, 1)',
                                    borderWidth: 1
                                },
                                {
                                    label: 'P(S|A)',
                                    data: barData.dataA,
                                    backgroundColor: 'rgba(255, 99, 132, 0.8)',
                                    borderColor: 'rgba(255, 99, 132, 1)',
                                    borderWidth: 1
                                },
                                {
                                    label: 'P(S|D)',
                                    data: barData.dataD,
                                    backgroundColor: 'rgba(75, 192, 192, 0.8)',
                                    borderColor: 'rgba(75, 192, 192, 1)',
                                    borderWidth: 1
                                }
                            ]
                        }
                    }
                };
                const chartCumulativeData = createFixedCumulativeChartData(
                    graph.rawDataH,
                    graph.rawDataA,
                    graph.rawDataD,
                    graph.rawDataPS,
                    graph.avgH,
                    graph.avgA,
                    graph.avgD,
                    graph.avgPS,
                    0.1
                );
                const integralColor = getColorForIntegral(chartCumulativeData.integralPS);
                overviewHTML += `
                            <div class="chart-card" style="background: linear-gradient(135deg, ${integralColor}15 0%, ${integralColor}25 100%); border-left: 4px solid ${integralColor};" data-cumulative="round_${round}_game_${game}_cumulative.html" data-bar="round_${round}_game_${game}_bar.html" data-integral="${chartCumulativeData.integralPS}" data-fc="${graph.totalYPSAboveZero}">
                <h3>Round ${round}, Game ${game}</h3>
                <div class="chart-preview-container" style="width:250px;height:120px;margin-bottom:10px;display:flex;align-items:center;justify-content:center;">
                    <canvas id="${chartId}" width="220" height="100"></canvas>
                </div>
                <div class="stats">
                    <div class="stat-row" style="font-weight: bold; font-size: 14px; color: #333;">
                        <div><strong>FS(x): ${(() => {
                            const cumulativeData = createFixedCumulativeChartData(
                                graph.rawDataH,
                                graph.rawDataA,
                                graph.rawDataD,
                                graph.rawDataPS,
                                graph.avgH,
                                graph.avgA,
                                graph.avgD,
                                graph.avgPS,
                                0.1
                            );
                            return cumulativeData.integralPS ? cumulativeData.integralPS.toFixed(3) : 'N/A';
                        })()}</strong></div>
                        <div><strong>FC(x): ${graph.totalYPSAboveZero ? graph.totalYPSAboveZero.toFixed(3) : 'N/A'}</strong></div>
                    </div>
                    <div class="stat-row">
                        <div data-original="${graph.avgPS || 0}">Eg[p]: ${graph.avgPS ? graph.avgPS.toFixed(3) : 'N/A'}</div>
                        <div>Eg[p | e0]: ${graph.avgH.toFixed(3)}</div>
                    </div>
                    <div class="stat-row">
                        <div>Eg[p | e1]: ${graph.avgA.toFixed(3)}</div>
                        <div>Eg[p | e2]: ${graph.avgD.toFixed(3)}</div>
                    </div>
                </div>
                <div class="click-hint">Click to view details</div>
            </div>`;
                chartIndex++;
            }
        });
    });
    overviewHTML += `
        </div>
    </div>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <script>
    // Chart data for toggling
    const chartDataMap = ${JSON.stringify(chartDataMap)};
    const chartInstances = {};
    let currentOffset = 0;
    let currentType = 'cumulative';
    let currentColorMetric = 'FS';

    function applyOffsetToData(data, offset) {
        return data.map(point => ({ x: point.x - offset, y: point.y }));
    }

    function renderChart(chartId, type, offset = 0) {
        const ctx = document.getElementById(chartId).getContext('2d');
        if (chartInstances[chartId]) {
            chartInstances[chartId].destroy();
        }
        const config = chartDataMap[chartId][type];
        
        // Apply offset to all datasets
        const offsetData = {
            datasets: config.data.datasets.map(dataset => ({
                ...dataset,
                data: applyOffsetToData(dataset.data, offset)
            }))
        };

        // Keep fixed x-axis ranges regardless of offset
        const xAxisRange = type === 'bar' ? { min: -1, max: 1 } : { min: 0, max: 1 };

        chartInstances[chartId] = new Chart(ctx, {
            type: config.type,
            data: offsetData,
            options: {
                responsive: false,
                maintainAspectRatio: false,
                plugins: {
                    legend: { display: false },
                    title: { display: false }
                },
                scales: {
                    x: { type: 'linear', ...xAxisRange, title: { display: false } },
                    y: { min: 0, max: 1, title: { display: false } }
                }
            }
        });
    }

    function applyOverviewOffset() {
        currentOffset = parseFloat(document.getElementById('overviewXOffset').value) || 0;
        Object.keys(chartDataMap).forEach(chartId => {
            renderChart(chartId, currentType, currentOffset);
        });
        
        // Update P(S) values and integral values in chart cards with offset
        const chartCards = document.querySelectorAll('.chart-card');
        let adjustedFSValues = [];
        let adjustedFCValues = [];
        chartCards.forEach((card, index) => {
            // Update P(S) Average with offset
            const secondStatRow = card.querySelectorAll('.stat-row')[1]; // Second row
            if (secondStatRow) {
                const psAvgDiv = secondStatRow.querySelectorAll('div')[0]; // First div in the second row
                if (psAvgDiv && psAvgDiv.textContent.includes('Eg[p]:')) {
                    const originalValue = parseFloat(psAvgDiv.getAttribute('data-original')) || 0;
                    const adjustedPS = originalValue - currentOffset;
                    psAvgDiv.innerHTML = 'Eg[p]: ' + adjustedPS.toFixed(3);
                }
            }
            
            // Update integral values and collect them for color calculation
            // The P(S) integral is now in the first stat row (top row), first div
            const firstStatRow = card.querySelectorAll('.stat-row')[0]; // First row
            if (firstStatRow) {
                const integralDiv = firstStatRow.querySelectorAll('div')[0]; // First div in the first row
                if (integralDiv && integralDiv.textContent.includes('FS(x):')) {
                    // Recalculate FS(x) using original normalized raw data (not cumulative)
                    const chartId = card.querySelector('canvas').id;
                    const chartData = chartDataMap[chartId];
                    if (chartData && chartData.rawDataPS) {
                        // Apply offset to raw data
                        const shiftedData = chartData.rawDataPS.map(point => ({ x: point.x - currentOffset, y: point.y }));
                        
                        let weightedSum = 0;
                        shiftedData.forEach(point => {
                            const p = point.x; // P(S) - offset (already shifted)
                            const normalizedFC = point.y; // FC/totalFC (normalized weight)
                            if (p > 0) {
                                const pClamped = Math.min(1, p);
                                weightedSum += normalizedFC * pClamped;
                            }
                        });
                        const newIntegral = weightedSum;
                        adjustedFSValues.push(newIntegral);
                        integralDiv.innerHTML = '<strong>FS(x): ' + newIntegral.toFixed(3) + '</strong>';
                    }
                }
                
                // Update FC(x) values with offset (second div in the first row)
                const fcDiv = firstStatRow.querySelectorAll('div')[1]; // Second div in the first row
                if (fcDiv && fcDiv.textContent.includes('FC(x):')) {
                    // Calculate new FC(x) with offset using original raw data
                    const chartId = card.querySelector('canvas').id;
                    const chartData = chartDataMap[chartId];
                    if (chartData && chartData.rawDataPS) {
                        // Use original raw data for accurate FC(x) calculation
                        // FC(x) = sum(FC where P(S) >= x) / sum(FC)
                        const newFC = chartData.rawDataPS
                            .filter(point => point.x - currentOffset >= 0)
                            .reduce((sum, point) => sum + point.y, 0);
                        
                        adjustedFCValues.push(newFC);
                        fcDiv.innerHTML = '<strong>FC(x): ' + newFC.toFixed(3) + '</strong>';
                    }
                }
            }
        });
        
        // Update average FS and FC
        const avgFSEl = document.getElementById('avgFS');
        const avgFCEl = document.getElementById('avgFC');
        if (avgFSEl && adjustedFSValues.length > 0) {
            const avgFS = adjustedFSValues.reduce((a,b)=>a+b,0) / adjustedFSValues.length;
            avgFSEl.textContent = avgFS.toFixed(3);
        }
        if (avgFCEl && adjustedFCValues.length > 0) {
            const avgFC = adjustedFCValues.reduce((a,b)=>a+b,0) / adjustedFCValues.length;
            avgFCEl.textContent = avgFC.toFixed(3);
        }

        // Update summary values and recalculate colors based on selected metric values
        const valuesForColor = currentColorMetric === 'FC' ? adjustedFCValues : adjustedFSValues;
        if (valuesForColor.length > 0) {
            const newMaxVal = Math.max(...valuesForColor);
            const newMinVal = Math.min(...valuesForColor);
            const newRange = newMaxVal - newMinVal;
            
            // Update header summary labels, values, and locations based on selected metric
            const highestLabel = document.getElementById('highestLabel');
            const lowestLabel = document.getElementById('lowestLabel');
            const highestValEl = document.getElementById('highestIntegral');
            const lowestValEl = document.getElementById('lowestIntegral');
            const highestLocEl = document.getElementById('highestIntegralLocation');
            const lowestLocEl = document.getElementById('lowestIntegralLocation');

            if (highestLabel && lowestLabel && highestValEl && lowestValEl && highestLocEl && lowestLocEl) {
                const metricLabel = currentColorMetric === 'FC' ? 'FC(x):' : 'FS(x):';
                highestLabel.textContent = 'Highest ' + metricLabel;
                lowestLabel.textContent = 'Lowest ' + metricLabel;
                highestValEl.textContent = newMaxVal.toFixed(3);
                lowestValEl.textContent = newMinVal.toFixed(3);

                // Determine locations (round/game) using card order alignment
                let maxIndex = valuesForColor.indexOf(newMaxVal);
                let minIndex = valuesForColor.indexOf(newMinVal);
                const maxCard = chartCards[maxIndex];
                const minCard = chartCards[minIndex];
                const idToRoundGame = (card) => {
                    if (!card) return '';
                    const title = card.querySelector('h3');
                    return title ? '(' + title.textContent + ')' : '';
                };
                highestLocEl.textContent = idToRoundGame(maxCard);
                lowestLocEl.textContent = idToRoundGame(minCard);
            }
            
            // Update chart card colors based on new integral values
            chartCards.forEach((card, index) => {
                if (index < valuesForColor.length) {
                    const adjustedVal = valuesForColor[index];
                    
                    // Calculate new color based on adjusted integral value
                    let integralColor = '#f0f0f0';
                    if (!isNaN(adjustedVal) && newRange > 0) {
                        const normalized = (adjustedVal - newMinVal) / newRange;
                        const red = Math.round(255 * normalized);
                        const green = Math.round(255 * (1 - normalized));
                        integralColor = 'rgb(' + red + ', ' + green + ', 0)';
                    }
                    
                    // Update card background color
                    card.style.background = 'linear-gradient(135deg, ' + integralColor + '15 0%, ' + integralColor + '25 100%)';
                    card.style.borderLeft = '4px solid ' + integralColor;
                }
            });
        }
    }

    // Initial render (cumulative)
    window.addEventListener('DOMContentLoaded', () => {
        Object.keys(chartDataMap).forEach(chartId => {
            renderChart(chartId, 'cumulative', 0);
        });
        
        const toggleBtn = document.getElementById('togglePreview');
        toggleBtn.addEventListener('change', () => {
            currentType = toggleBtn.value;
            Object.keys(chartDataMap).forEach(chartId => {
                renderChart(chartId, currentType, currentOffset);
            });
        });

        const colorSelect = document.getElementById('colorMetricSelect');
        colorSelect.addEventListener('change', () => {
            currentColorMetric = colorSelect.value;
            // Update summary labels and values according to selected metric
            const highestLabel = document.getElementById('highestLabel');
            const lowestLabel = document.getElementById('lowestLabel');
            const highestValEl = document.getElementById('highestIntegral');
            const lowestValEl = document.getElementById('lowestIntegral');
            const highestLocEl = document.getElementById('highestIntegralLocation');
            const lowestLocEl = document.getElementById('lowestIntegralLocation');

            if (currentColorMetric === 'FC') {
                highestLabel.textContent = 'Highest FC(x):';
                lowestLabel.textContent = 'Lowest FC(x):';

                // Compute FC max/min across cards (at current offset)
                const chartCards = Array.from(document.querySelectorAll('.chart-card'));
                const fcValues = chartCards.map(card => {
                    const chartId = card.querySelector('canvas').id;
                    const chartData = chartDataMap[chartId];
                    if (!chartData || !chartData.rawDataPS) return NaN;
                    const newFC = chartData.rawDataPS
                        .filter(point => point.x - currentOffset >= 0)
                        .reduce((sum, point) => sum + point.y, 0);
                    return newFC;
                }).filter(v => !isNaN(v));
                if (fcValues.length > 0) {
                    const maxFC = Math.max(...fcValues);
                    const minFC = Math.min(...fcValues);
                    highestValEl.textContent = maxFC.toFixed(3);
                    lowestValEl.textContent = minFC.toFixed(3);

                    // Find locations for highest/lowest FC
                    let maxIndex = fcValues.indexOf(maxFC);
                    let minIndex = fcValues.indexOf(minFC);
                    const maxCard = chartCards[maxIndex];
                    const minCard = chartCards[minIndex];
                    const idToRoundGame = (card) => {
                        if (!card) return '';
                        const title = card.querySelector('h3');
                        return title ? '(' + title.textContent + ')' : '';
                    };
                    highestLocEl.textContent = idToRoundGame(maxCard);
                    lowestLocEl.textContent = idToRoundGame(minCard);
                }
            } else {
                highestLabel.textContent = 'Highest FS(x):';
                lowestLabel.textContent = 'Lowest FS(x):';

                // Recompute FS max/min at current offset
                const chartCards = Array.from(document.querySelectorAll('.chart-card'));
                const fsValues = chartCards.map(card => {
                    const chartId = card.querySelector('canvas').id;
                    const chartData = chartDataMap[chartId];
                    if (!chartData || !chartData.rawDataPS) return NaN;
                    const shiftedData = chartData.rawDataPS.map(point => ({ x: point.x - currentOffset, y: point.y }));
                    let weightedSum = 0;
                    shiftedData.forEach(point => {
                        const p = point.x;
                        const w = point.y;
                        if (p > 0) {
                            const pClamped = Math.min(1, p);
                            weightedSum += w * pClamped;
                        }
                    });
                    return weightedSum;
                }).filter(v => !isNaN(v));

                if (fsValues.length > 0) {
                    const maxFS = Math.max(...fsValues);
                    const minFS = Math.min(...fsValues);
                    highestValEl.textContent = maxFS.toFixed(3);
                    lowestValEl.textContent = minFS.toFixed(3);

                    // Find locations for highest/lowest FS
                    let maxIndex = fsValues.indexOf(maxFS);
                    let minIndex = fsValues.indexOf(minFS);
                    const maxCard = chartCards[maxIndex];
                    const minCard = chartCards[minIndex];
                    const idToRoundGame = (card) => {
                        if (!card) return '';
                        const title = card.querySelector('h3');
                        return title ? '(' + title.textContent + ')' : '';
                    };
                    highestLocEl.textContent = idToRoundGame(maxCard);
                    lowestLocEl.textContent = idToRoundGame(minCard);
                }
            }

            // Recompute and recolor with the currently selected metric
            applyOverviewOffset();
        });

        // Add enter key support for the offset input
        document.getElementById('overviewXOffset').addEventListener('keypress', function(e) {
            if (e.key === 'Enter') {
                applyOverviewOffset();
            }
        });
        
        // Add click handlers for chart cards
        const chartCards = document.querySelectorAll('.chart-card');
        chartCards.forEach(card => {
            card.addEventListener('click', () => {
                const currentType = document.getElementById('togglePreview').value;
                const href = currentType === 'cumulative' ? 
                    card.getAttribute('data-cumulative') : 
                    card.getAttribute('data-bar');
                
                if (href) {
                    window.open(href, '_blank');
                }
            });
        });
    });
    </script>
</body>
</html>`;

    fs.writeFileSync(`${dir}/overview.html`, overviewHTML);
}

// Main visualization function
function createVisualizations(fileName = 'output.csv') {
    try {
        console.log('Starting HTML visualization generation...');
        
        // Read and parse CSV file
        const csvData = fs.readFileSync(fileName, 'utf8')
            .split('\n')
            .map(row => row.split(',').map(cell => cell.trim()));

        // Remove header row
        const headers = csvData.shift();

        // Combine the data
        const combinedData = combineRowsByRoundAndGame(csvData, headers);

        // Find unique rounds and games
        const rounds = new Set();
        const games = new Set();
        combinedData.forEach(row => {
            if (row.length >= 2) {
                rounds.add(parseInt(row[0]));
                games.add(parseInt(row[1]));
            }
        });

        // Convert sets to sorted arrays
        const sortedRounds = Array.from(rounds).sort((a, b) => a - b);
        const sortedGames = Array.from(games).sort((a, b) => a - b);

        // Process and create individual graphs
        const allGraphs = [];

        sortedRounds.forEach(round => {
            sortedGames.forEach(game => {
                const { bucketDataH, bucketDataA, bucketDataD, rawDataH, rawDataA, rawDataD, rawDataPS, avgH, avgA, avgD, avgPS, totalYPSAboveZero, fsX } = processData(round, game, csvData);
                if (bucketDataH.length > 0 || bucketDataA.length > 0 || bucketDataD.length > 0) {
                    createHTMLVisualization(round, game, bucketDataH, bucketDataA, bucketDataD, rawDataH, rawDataA, rawDataD, rawDataPS, avgH, avgA, avgD, avgPS, totalYPSAboveZero, fsX);
                    allGraphs.push({ round, game, bucketDataH, bucketDataA, bucketDataD, rawDataH, rawDataA, rawDataD, rawDataPS, avgH, avgA, avgD, avgPS, totalYPSAboveZero, fsX });
                    console.log(`Created HTML visualizations for Round ${round}, Game ${game}`);
                }
            });
        });

        // Create combined overview
        createCombinedOverview(allGraphs, sortedRounds, sortedGames);

        console.log('All HTML visualizations have been generated!');
        console.log(`Visualizations saved to: ./visualizations/`);
        console.log(`Open ./visualizations/overview.html in your browser to view all charts.`);
        
    } catch (error) {
        console.error('Error creating visualizations:', error.message);
        throw error;
    }
}

module.exports = { createVisualizations }; 