// PrismExport addr=0x180047140 name=guess ghidra=FUN_180047140 status=clean lines=146

void FUN_180047140(longlong param_1,ulonglong param_2)

{
  byte *pbVar1;
  longlong *plVar2;
  longlong *plVar3;
  ulonglong _Size;
  longlong *plVar4;
  undefined8 *puVar5;
  undefined8 *puVar6;
  longlong *plVar7;
  int iVar8;
  ulonglong uVar9;
  ulonglong uVar10;
  longlong *plVar11;
  ulonglong uVar12;
  longlong *plVar13;
  longlong lVar14;
  longlong *plVar15;
  undefined8 *puVar16;
  
  for (lVar14 = 0x3f; 0xfffffffffffffffU >> lVar14 == 0; lVar14 = lVar14 + -1) {
  }
  if ((ulonglong)(1L << ((byte)lVar14 & 0x3f)) < param_2) {
                    /* WARNING: Subroutine does not return */
    std::_Xlength_error("invalid hash bucket count");
  }
  plVar2 = *(longlong **)(param_1 + 8);
  uVar9 = param_2 - 1 | 1;
  lVar14 = 0x3f;
  if (uVar9 != 0) {
    for (; uVar9 >> lVar14 == 0; lVar14 = lVar14 + -1) {
    }
  }
  lVar14 = 1L << ((char)lVar14 + 1U & 0x3f);
  FUN_180046db0((ulonglong *)(param_1 + 0x18),lVar14 * 2,plVar2);
  *(longlong *)(param_1 + 0x38) = lVar14;
  *(longlong *)(param_1 + 0x30) = lVar14 + -1;
  plVar7 = (longlong *)**(undefined8 **)(param_1 + 8);
joined_r0x0001800471c4:
  do {
    while( true ) {
      if (plVar7 == plVar2) {
        return;
      }
      uVar9 = plVar7[5];
      plVar15 = plVar7 + 2;
      plVar3 = (longlong *)*plVar7;
      _Size = plVar7[4];
      if (0xf < uVar9) {
        plVar15 = (longlong *)plVar7[2];
      }
      uVar10 = 0;
      uVar12 = 0xcbf29ce484222325;
      if (_Size != 0) {
        do {
          pbVar1 = (byte *)((longlong)plVar15 + uVar10);
          uVar10 = uVar10 + 1;
          uVar12 = (uVar12 ^ *pbVar1) * 0x100000001b3;
        } while (uVar10 < _Size);
      }
      puVar16 = (undefined8 *)
                ((*(ulonglong *)(param_1 + 0x30) & uVar12) * 0x10 + *(longlong *)(param_1 + 0x18));
      plVar15 = (longlong *)*puVar16;
      if (plVar15 != plVar2) break;
      *puVar16 = plVar7;
      puVar16[1] = plVar7;
      plVar7 = plVar3;
    }
    plVar4 = (longlong *)puVar16[1];
    plVar13 = plVar4 + 2;
    if (0xf < (ulonglong)plVar4[5]) {
      plVar13 = (longlong *)*plVar13;
    }
    plVar11 = plVar7 + 2;
    if (0xf < uVar9) {
      plVar11 = (longlong *)plVar7[2];
    }
    if (_Size != plVar4[4]) {
joined_r0x0001800472c9:
      while (plVar15 != plVar4) {
        plVar4 = (longlong *)plVar4[1];
        plVar13 = plVar4 + 2;
        if (0xf < (ulonglong)plVar4[5]) {
          plVar13 = (longlong *)*plVar13;
        }
        if (uVar9 < 0x10) {
          plVar11 = plVar7 + 2;
        }
        else {
          plVar11 = (longlong *)plVar7[2];
        }
        if (_Size == plVar4[4]) {
          if ((_Size == 0) || (iVar8 = memcmp(plVar11,plVar13,_Size), iVar8 == 0)) {
            lVar14 = *plVar4;
            plVar15 = (longlong *)plVar7[1];
            *plVar15 = (longlong)plVar3;
            plVar13 = (longlong *)plVar3[1];
            *plVar13 = lVar14;
            puVar16 = *(undefined8 **)(lVar14 + 8);
            *puVar16 = plVar7;
            *(longlong **)(lVar14 + 8) = plVar13;
            plVar3[1] = (longlong)plVar15;
            plVar7[1] = (longlong)puVar16;
            plVar7 = plVar3;
            goto joined_r0x0001800471c4;
          }
          uVar9 = plVar7[5];
        }
      }
      plVar15 = (longlong *)plVar7[1];
      *plVar15 = (longlong)plVar3;
      puVar5 = (undefined8 *)plVar3[1];
      *puVar5 = plVar4;
      puVar6 = (undefined8 *)plVar4[1];
      *puVar6 = plVar7;
      plVar4[1] = (longlong)puVar5;
      plVar3[1] = (longlong)plVar15;
      plVar7[1] = (longlong)puVar6;
      *puVar16 = plVar7;
      plVar7 = plVar3;
      goto joined_r0x0001800471c4;
    }
    if ((_Size != 0) && (iVar8 = memcmp(plVar11,plVar13,_Size), iVar8 != 0)) {
      uVar9 = plVar7[5];
      goto joined_r0x0001800472c9;
    }
    plVar4 = (longlong *)*plVar4;
    if (plVar4 != plVar7) {
      plVar15 = (longlong *)plVar7[1];
      *plVar15 = (longlong)plVar3;
      puVar5 = (undefined8 *)plVar3[1];
      *puVar5 = plVar4;
      puVar6 = (undefined8 *)plVar4[1];
      *puVar6 = plVar7;
      plVar4[1] = (longlong)puVar5;
      plVar3[1] = (longlong)plVar15;
      plVar7[1] = (longlong)puVar6;
    }
    puVar16[1] = plVar7;
    plVar7 = plVar3;
  } while( true );
}

