# AMX for gem5
Research project implementing Intel AMX features into gem5 to profile matrix multiplication and LLM inference workloads.

## Usage

### 1. Compile C++ Workloads

``` bash
make
```

### 2. Build gem5 & m5ops


i) gem5: 
```bash 
cd gem5 && scons build/{ISA}/gem5.{variant} -j {cpus}
```

ii) m5ops: 
```bash
cd util/m5 && scons build/{TARGET_ISA}/out/m5 '
```

### 3. Run Simulation

``` bash
./path-to-gem5-build -rs amx/tb.py
```
