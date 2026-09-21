// PrismExport addr=0x18002e340 name=VersionPacker ghidra=SKSEPlugin_Query status=clean lines=16

undefined8 SKSEPlugin_Query(undefined8 param_1,undefined4 *param_2)

{
  uint uVar1;
  
                    /* 0x2e340  2  SKSEPlugin_Query */
  *param_2 = 1;
  *(char **)(param_2 + 2) = s_SexLabPPrism_180095248;
  uVar1 = DAT_180095244 >> 4 & 0xfff;
  param_2[4] = ((DAT_180095244 >> 0x10 & 0xff | (DAT_180095244 >> 0x18) << 8) << 0xc | uVar1) << 4 |
               DAT_180095244 & 0xf;
  return CONCAT71((uint7)(uint3)(uVar1 >> 8),1);
}

