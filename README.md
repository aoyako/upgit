# UPGIT
Upload local files to a git repositiry

### Installation
1. `mkdir build && cd build`
2. `cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..`
3. `cmake --build . --config Release --target install`

### Running
`upgit /path/to/config`

### Configuration format
Space separated text file in the following format. Each line must contain exactly 3 columns:
| Column 1 | Column 2 | Column 3 |
|-----------------|-----------------|-----------------|
| Path to a local folder where git repository will be cached | URL to a git repository | Path to a local file or folder that will be copied to the repository  |
