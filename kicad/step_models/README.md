# Component STEP models

The [Keystone 1290-79 holder](Keystone_1290-79_drawing-based_clanker.step) for BT1 is included in this repository. It is a project-generated approximation based on the [Keystone mechanical drawing](https://media.soselectronic.com/KatList/keystone/1290-79.pdf). Project-generated STEP filenames end in `_clanker`.

To restore the other models after cloning, download each file into this folder and save it with the exact filename shown. Git ignores these downloaded files. KiCad resolves them relative to the project, so you can open the PCB without configuring a model search path.

| PCB references | Part | Save as | STEP download |
| --- | --- | --- | --- |
| J1 | GCT USB4105-GF-A USB-C receptacle | `USB4105.step` | [KiCad 10.0.0 library](https://gitlab.com/kicad/libraries/kicad-packages3D/-/raw/10.0.0/Connector_USB.3dshapes/USB_C_Receptacle_GCT_USB4105-xx-A_16P_TopMnt_Horizontal.step) |
| J2 | Kinghelm KH-BNC50-3511 BNC jack | `KH-BNC50-3511.step` | [EasyEDA model](https://modules.easyeda.com/qAxj6KHrDKw4blvCG8QJPs7Y/0dcde789bcd54cae9917a7278df9ba96) |
| J3 | Samtec FTSH-105-01-L-DV-007-TR SWD header, pin 7 omitted | `Samtec_FTSH-105-xx-x-DV-007.step` | [KiCad contributor model, pinned revision](https://gitlab.com/kicad/libraries/kicad-packages3D/-/raw/3872bc52638ecad60eb75f96a9694a703af38a29/Connector_Samtec_FTSH.3dshapes/Samtec_FTSH-105-xx-x-DV-007.step) |
| SW1 | Nidec CSS-1210TB slide switch | `CSS-1210TB.step` | [EasyEDA model](https://modules.easyeda.com/qAxj6KHrDKw4blvCG8QJPs7Y/e6870ee571e74bea9c692e16cd3450b2) |
| D1–D9 | Worldsemi WS2812B-2020-V6 RGB LED | `WS2812B-2020-V6.step` | [EasyEDA package model](https://modules.easyeda.com/qAxj6KHrDKw4blvCG8QJPs7Y/79fb709ea16a48209cb9819d55740b7e) |
| TP5–TP12, TP15–TP26 | Harwin S1751-46 SMT test point | `Harwin_S1751-46.step` | [Harwin model mirrored by the LWA project](https://raw.githubusercontent.com/lwa-project/front_end/e429f8fa02577258fa76e683ea872ce53ddebf53/LWA%20Balun%202020/3D%20Models/s1751-46r.stp) |
