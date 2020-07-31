Windows Filtering Platform Demo.
Filter in Steam And ALE layer to get local ip and port.

#### include <fwpsk.h>
```
#pragma warning(push)
#pragma warning(disable: 4201)	
#include <fwpsk.h>				
#pragma warning(pop)			
```
#### add process definitions
```
Project Settings -> C/C++ -> Preprocessor -> Preprocessor definitions
add -> NDIS_SUPPORT_NDIS6
```

### add link Dependence
```
$(DDK_LIB_PATH)\ndis.lib
$(DDK_LIB_PATH)\wdmsec.lib
$(DDK_LIB_PATH)\fwpkclnt.lib
$(SDK_LIB_PATH)\uuid.lib
```

