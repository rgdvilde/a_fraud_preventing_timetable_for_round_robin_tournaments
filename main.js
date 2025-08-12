#!/usr/bin/env node

const { spawn } = require('child_process');
const { dirname, join } = require('path');
const { chmod, mkdtemp, writeFile, readFile } = require('fs/promises');
const fs = require('fs');
const os = require('os');

// Check if we're running in a pkg bundle
const isPkg = !!process.pkg;

// Path to the thesis binary - try multiple possible locations
function findBinaryPath() {
  const possiblePaths = [
    join(__dirname, 'cpp', 'cmake-build-debug', 'thesis'),  // C++ build directory
    join(__dirname, 'thesis'),                              // Development
    join(__dirname, 'snapshot', 'thesis_jswrapper', 'thesis'), // Packaged
    join(__dirname, 'thesis_jswrapper'),                    // Alternative packaged path
    join(process.cwd(), 'thesis')                           // Current working directory
  ];
  
  for (const path of possiblePaths) {
    if (fs.existsSync(path)) {
      return path;
    }
  }
  
  throw new Error('Could not find thesis binary in any expected location');
}

async function getExecutablePath() {
  if (isPkg) {
    const sourcePath = findBinaryPath();
    const tmpDir = await mkdtemp(join(os.tmpdir(), 'thesis-bin-'));
    const targetPath = join(tmpDir, 'thesis');
    
    const buffer = await readFile(sourcePath);
    await writeFile(targetPath, buffer, { mode: 0o755 }); // ensure executable
    
    return targetPath;
  } else {
    const binaryPath = findBinaryPath();
    await makeBinaryExecutable(binaryPath);
    return binaryPath;
  }
}

async function makeBinaryExecutable(binaryPath) {
  try {
    await chmod(binaryPath, 0o755);
  } catch (error) {
    console.warn('Warning: Could not set executable permissions on thesis binary:', error.message);
  }
}

function runBinary(binaryPath) {
  return new Promise((resolve, reject) => {
    // Get all command line arguments (skip the first two: node and script name)
    const args = process.argv.slice(2);
        
    // Spawn the binary process
    const child = spawn(binaryPath, args, {
      stdio: 'inherit', // Pass through stdin, stdout, stderr
      cwd: process.cwd() // Use current working directory
    });
    
    // Handle process events
    child.on('error', (error) => {
      console.error('Error executing binary:', error.message);
      reject(error);
    });
    
    child.on('close', (code) => {
      if (code === 0) {
        console.info('Thesis binary completed successfully');
        resolve();
      } else {
        console.error(`Thesis binary exited with code ${code}`);
        reject(new Error(`Binary exited with code ${code}`));
      }
    });
    
    // Handle process termination signals
    process.on('SIGINT', () => {
      child.kill('SIGINT');
    });
    
    process.on('SIGTERM', () => {
      child.kill('SIGTERM');
    });
  });
}

// Function to find output file from command line arguments
function findOutputFile(args) {
  // Look for -o flag and get the next argument as the output file
  const outputIndex = args.indexOf('-o');
  if (outputIndex !== -1 && outputIndex + 1 < args.length) {
    return args[outputIndex + 1];
  }
  
  // Default to output.csv if no -o flag found
  return 'output.csv';
}

// Main execution
async function main() {
  try {
    const binaryPath = await getExecutablePath();
    await runBinary(binaryPath);
    
    // Check if -v flag is present for visualization
    const args = process.argv.slice(2);
    const shouldVisualize = args.includes('-v');
    
    if (shouldVisualize) {
      // After binary completes successfully, run visualizations
      console.info('Starting visualization generation...');
      
      // Find the actual output file from command line arguments
      const outputFile = findOutputFile(args);
      if (!fs.existsSync(outputFile)) {
        console.warn(`Warning: ${outputFile} not found. Skipping visualization.`);
        return;
      }
      
      // Run HTML-based visualizations
      try {
        // Import visualization module
        const { createVisualizations } = require('./visualization-html.js');
        createVisualizations(outputFile);
        console.info('✅ HTML visualization process completed successfully!');
        console.info('🌐 Open ./visualizations/overview.html in your browser to view all charts.');
        
      } catch (error) {
        console.error('❌ Error during visualization:', error.message);
      }
    }
    
  } catch (error) {
    console.error('Fatal error:', error.message);
    process.exit(1);
  }
}

// Run the main function
main();
