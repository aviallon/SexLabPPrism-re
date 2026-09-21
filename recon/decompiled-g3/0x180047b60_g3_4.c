// PrismExport addr=0x180047b60 name=g3_4 ghidra=FUN_180047b60 status=clean lines=163

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */
/* WARNING: Type propagation algorithm not settling */

undefined8 FUN_180047b60(undefined8 *param_1)

{
  code *pcVar1;
  int iVar2;
  char *******pppppppcVar3;
  size_t sVar4;
  ulonglong uVar5;
  undefined4 extraout_var;
  char *******extraout_RAX;
  ulonglong extraout_RAX_00;
  char *******extraout_RAX_01;
  undefined8 uVar6;
  undefined8 *puVar7;
  byte *pbVar8;
  ulonglong uVar9;
  undefined1 (*pauVar10) [16];
  undefined1 auStack_1d8 [32];
  byte local_1b8;
  char local_1a8 [256];
  char *******local_a8;
  undefined8 uStack_a0;
  longlong local_98;
  ulonglong local_90;
  undefined1 local_88 [48];
  ulonglong local_58;
  
  local_58 = DAT_180095900 ^ (ulonglong)auStack_1d8;
  puVar7 = param_1;
  if (0xf < (ulonglong)param_1[3]) {
    puVar7 = (undefined8 *)*param_1;
  }
  pppppppcVar3 = (char *******)_stat64i32(puVar7,local_88);
  if ((int)pppppppcVar3 != 0) {
    uVar5 = param_1[2];
    if (uVar5 == 0) {
LAB_180047e0e:
      return (ulonglong)pppppppcVar3 & 0xffffffffffffff00;
    }
    uVar9 = 0;
    do {
      sVar4 = strlen("\\/");
      puVar7 = param_1;
      if (0xf < (ulonglong)param_1[3]) {
        puVar7 = (undefined8 *)*param_1;
      }
      if ((sVar4 == 0) || (uVar5 <= uVar9)) {
LAB_180047c82:
        uVar5 = param_1[2];
      }
      else {
        pauVar10 = (undefined1 (*) [16])(uVar9 + (longlong)puVar7);
        if (sVar4 + (uVar5 - uVar9) < 0x10) {
          memset(local_1a8,0,0x100);
          for (pbVar8 = &DAT_180061a4c; &DAT_180061a4c + sVar4 != pbVar8; pbVar8 = pbVar8 + 1) {
            local_1a8[*pbVar8] = '\x01';
          }
          for (; pauVar10 < (undefined1 (*) [16])(uVar5 + (longlong)puVar7);
              pauVar10 = (undefined1 (*) [16])(*pauVar10 + 1)) {
            if (local_1a8[(byte)(*pauVar10)[0]] != '\0') {
              uVar5 = (longlong)pauVar10 - (longlong)puVar7;
              goto LAB_180047c7c;
            }
          }
          goto LAB_180047c82;
        }
        uVar5 = FUN_18004fa40(pauVar10,uVar5 - uVar9,(undefined1 (*) [16])&DAT_180061a4c,sVar4);
        if (uVar5 != 0xffffffffffffffff) {
          uVar5 = uVar5 + uVar9;
        }
LAB_180047c7c:
        if (uVar5 == 0xffffffffffffffff) goto LAB_180047c82;
      }
      local_a8 = (char *******)0x0;
      uStack_a0 = 0;
      local_98 = 0;
      local_90 = 0;
      uVar9 = uVar5;
      if ((ulonglong)param_1[2] < uVar5) {
        uVar9 = param_1[2];
      }
      puVar7 = param_1;
      if (0xf < (ulonglong)param_1[3]) {
        puVar7 = (undefined8 *)*param_1;
      }
      pppppppcVar3 = (char *******)FUN_180001670(&local_a8,puVar7,uVar9);
      if (local_98 == 2) {
        pppppppcVar3 = (char *******)&local_a8;
        if (0xf < local_90) {
          pppppppcVar3 = local_a8;
        }
        if (*(char *)((longlong)pppppppcVar3 + 1) == ':') {
          if (local_90 < 3) {
            pppppppcVar3 = (char *******)FUN_180007dd0(&local_a8,1,(ulonglong)local_1b8,0x5c);
          }
          else {
            local_98 = 3;
            pppppppcVar3 = (char *******)&local_a8;
            if (0xf < local_90) {
              pppppppcVar3 = local_a8;
            }
            ((char *)((longlong)pppppppcVar3 + 2))[0] = '\\';
            ((char *)((longlong)pppppppcVar3 + 2))[1] = '\0';
          }
          uVar5 = uVar5 + 1;
        }
      }
      if (local_98 != 0) {
        pppppppcVar3 = (char *******)&local_a8;
        if (0xf < local_90) {
          pppppppcVar3 = local_a8;
        }
        pppppppcVar3 = (char *******)_stat64i32(pppppppcVar3,local_88);
        if ((int)pppppppcVar3 != 0) {
          pppppppcVar3 = (char *******)&local_a8;
          if (0xf < local_90) {
            pppppppcVar3 = local_a8;
          }
          iVar2 = _mkdir((char *)pppppppcVar3);
          pppppppcVar3 = (char *******)CONCAT44(extraout_var,iVar2);
          if (iVar2 != 0) {
            if (0xf < local_90) {
              if (0xfff < local_90 + 1) {
                if ((char *)((longlong)local_a8 + (-8 - (longlong)local_a8[-1])) < (char *)0x20) {
                  free(local_a8[-1]);
                  return extraout_RAX_00 & 0xffffffffffffff00;
                }
                goto LAB_180047e12;
              }
              free(local_a8);
              pppppppcVar3 = extraout_RAX_01;
            }
            goto LAB_180047e0e;
          }
        }
      }
      if (0xf < local_90) {
        pppppppcVar3 = local_a8;
        if ((0xfff < local_90 + 1) &&
           (pppppppcVar3 = (char *******)local_a8[-1],
           (char *)0x1f < (char *)((longlong)local_a8 + (-8 - (longlong)pppppppcVar3)))) {
LAB_180047e12:
          pcVar1 = (code *)swi(0x29);
          (*pcVar1)(5);
          pcVar1 = (code *)swi(3);
          uVar6 = (*pcVar1)();
          return uVar6;
        }
        free(pppppppcVar3);
        pppppppcVar3 = extraout_RAX;
      }
      uVar9 = uVar5 + 1;
      uVar5 = param_1[2];
    } while (uVar9 < uVar5);
  }
  return CONCAT71((int7)((ulonglong)pppppppcVar3 >> 8),1);
}

