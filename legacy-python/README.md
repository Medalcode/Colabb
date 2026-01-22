# Legacy Python Implementation

This directory contains the original Python implementation of Colabb Terminal.

**Status**: Deprecated - No longer maintained

**Reason**: The Python version had compatibility issues with VTE bindings that made it unreliable across different Linux distributions. The C++ rewrite solves these issues with direct API integration.

## Migration to C++

The new C++ implementation is located in `../colabb-cpp/` and provides:

- Better performance
- No binding compatibility issues
- Cleaner architecture
- Secure credential storage

Please use the C++ version for all new deployments.
