# A Fraud Preventing Timetable for Round Robin Tournaments

## Prerequisites

- Node.js 18+
- Yarn package manager
- CMake 3.10+
- C++ compiler (GCC, Clang, or MSVC)

## Building

### Build for All Platforms

```bash
yarn build
```

This command will:
1. Build the C++ code for your current platform
2. Package the application for macOS, Linux, and Windows

### Build for Specific Platform

#### macOS
```bash
yarn make:macos
yarn pkg:macos
```

#### Linux
```bash
yarn make:linux
yarn pkg:linux
```

#### Windows
```bash
yarn make:windows
yarn pkg:windows
```

## Usage

After building, the executables will be available in the `./dist/` directory.

### Command Format

```bash
./dist/thesis-[platform] -c [config_file] -o [output_file]
```

### Platform-Specific Commands

#### macOS
```bash
./dist/thesis-macos -c example_data/in.json -o test.csv
```

#### Linux
```bash
./dist/thesis-linux -c example_data/in.json -o test.csv
```

#### Windows
```bash
./dist/thesis-win.exe -c example_data/in.json -o test.csv
```

### Parameters

- `-c` or `--config`: Input configuration file (JSON format)
- `-o` or `--output`: Output CSV file path

### Example Configuration

The application expects a JSON configuration file. See `example_data/in.json` for the expected format.

## Project Structure

```
root/
├── cpp/                    # C++ source code
│   ├── calculation/        # Calculation algorithms
│   ├── optimization/       # Optimization strategies
│   ├── priceFunctions/     # Price function implementations
│   └── helpers/           # Utility functions
├── example_data/          # Sample data and configuration
├── dist/                  # Built executables (after build)
└── main.js               # JavaScript entry point
```
