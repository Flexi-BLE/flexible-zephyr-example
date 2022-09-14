# FlexiBLE Zephyr Example

An example project with FlexiBLE services on Zephyr RTOS. Designed to run on Nordic nRF53840 Development Kit ([DigiKey](https://www.digikey.com/en/product-highlight/n/nordic-semi/nrf52840-pdk-development-kit-for-nrf52840-soc?utm_adgroup=Battery%20Products&utm_source=bing&utm_medium=cpc&utm_campaign=Dynamic%20Search_EN_Product&utm_term=batteries&utm_content=Battery%20Products&utm_id=bi_cmp-420474294_adg-1297424083731330_ad-81089079962681_dat-2332888796383360:loc-190_dev-c_ext-_prd-&msclkid=ad8828b3ed3810aa0e7d4b921689505d)). Sensors data streams are simulated with random data.

## Helpful links
* [Nordic NRF53 Documentation](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fug_gsg_ncs%2FUG%2Fgsg%2Ffirst_test.html)
* [Zephyr Documentation](https://docs.zephyrproject.org/latest/) ([repo](https://github.com/zephyrproject-rtos/zephyr))

## Getting Started
* Install [Nordic nRF Connect for Desktop](https://www.nordicsemi.com/Products/Development-tools/nrf-connect-for-desktop/download)
* In nRF Connect: Install the Toolchain Manager and Current SDK (`v2.0.0`)
    * ⚠️ if installation hangs on "updaing repositories ...", quit the application then reopen and select update toolchain, then update sdk to continue.
* Install [Visual Studio Code](https://code.visualstudio.com/)
* In nRF Connect Toolchain Manager: select "Open VS Code"
    * must open VS code from nRF Connect Toolchain Manager (TODO: find out what command line flags are being passed to `code`).
* Install VS Code extensions if needed
* Follow the [Zephyr getting started guide](https://docs.zephyrproject.org/latest/develop/getting_started/index.html) to install all zephyr requirements (you can stop before `Flash the Sample`).
    * *optional*: setup Python in a virtual environment using Miniconda
      * Install [Miniconda](https://docs.conda.io/en/latest/miniconda.html) (recommended), or [Anaconda](https://www.anaconda.com/).
      * Create a new environment for zephyr `conda create -n zeph python` (the trailing `python` flag will install the lastest [Python](https://www.python.org/) version)
      * Active the new environment and proceed with Zepyth Getting Started guide `conda activate zeph`
* From the folder you wish to contain this repository (e.g. `C:\Projects`), run `west init -m [SSH or HTTP Clone URL] flexible-nrf53-sample`
* Change directory to `flexible-nrf53-sample` and run `west update`
* You can build the firmware with `west` or with the nRF VSCode extension. To use `west`, change directory to `flexible-nrf53-sample` and run `west build -b nrf5340dk_nrf5340_cpuapp_ns`. 
* To flash the firmware with `west`, you can simply type `west flash` after the build completes. To flash with vscode nrf extension, press the `Flash` button. 


## Known Issues & Gotchas
### C++ Language Support
The order of KConfigs matters when building with C++ standard library
```
CONFIG_CPLUSPLUS=y
CONFIG_LIB_CPLUSPLUS=y
CONFIG_NEWLIB_LIBC=y
```
Source: [Nordic Dev Zone](https://devzone.nordicsemi.com/f/nordic-q-a/81378/a-problem-with-zephyr-c-and-string)

### C++ Bluetooth Support
There is, at least, one bug with the Zephr Bluetooth library when building with C++:
```c++
// initializing the bluetooth
err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
```

results in the following compliation error:
```
../src/main.cpp: In function 'void main()':
C:/Users/320079877/ncs/v1.4.2/zephyr/include/bluetooth/bluetooth.h:591:30: error: taking address of temporary array
  591 |  ((struct bt_le_adv_param[]) { \
      |  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~^~~
  592 |   BT_LE_ADV_PARAM_INIT(_options, _int_min, _int_max, _peer) \
      |   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  593 |   })
      |   ~~
C:/Users/320079877/ncs/v1.4.2/zephyr/include/bluetooth/bluetooth.h:600:24: note: in expansion of macro 'BT_LE_ADV_PARAM'
  600 | #define BT_LE_ADV_CONN BT_LE_ADV_PARAM(BT_LE_ADV_OPT_CONNECTABLE, \
      |                        ^~~~~~~~~~~~~~~
../src/main.cpp:232:24: note: in expansion of macro 'BT_LE_ADV_CONN'
  232 |  err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad),
      |                        ^~~~~~~~~~~~~~
```
This can be resolved by channing line 591 from `struct` to `const`. Source: [Zephyr Github Issue](https://github.com/zephyrproject-rtos/zephyr/issues/18551)
