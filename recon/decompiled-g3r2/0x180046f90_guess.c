// PrismExport addr=0x180046f90 name=guess ghidra=FUN_180046f90 status=clean lines=101

void FUN_180046f90(longlong param_1,ulonglong param_2)

{
  undefined8 *puVar1;
  byte bVar2;
  longlong *plVar3;
  longlong *plVar4;
  longlong *plVar5;
  longlong *plVar6;
  undefined8 *puVar7;
  undefined8 *puVar8;
  longlong *plVar9;
  ulonglong uVar10;
  longlong lVar11;
  
  for (lVar11 = 0x3f; 0xfffffffffffffffU >> lVar11 == 0; lVar11 = lVar11 + -1) {
  }
  if ((ulonglong)(1L << ((byte)lVar11 & 0x3f)) < param_2) {
                    /* WARNING: Subroutine does not return */
    std::_Xlength_error("invalid hash bucket count");
  }
  plVar3 = *(longlong **)(param_1 + 8);
  uVar10 = param_2 - 1 | 1;
  lVar11 = 0x3f;
  if (uVar10 != 0) {
    for (; uVar10 >> lVar11 == 0; lVar11 = lVar11 + -1) {
    }
  }
  lVar11 = 1L << ((char)lVar11 + 1U & 0x3f);
  FUN_180046db0((ulonglong *)(param_1 + 0x18),lVar11 * 2,plVar3);
  *(longlong *)(param_1 + 0x38) = lVar11;
  *(longlong *)(param_1 + 0x30) = lVar11 + -1;
  plVar9 = (longlong *)**(undefined8 **)(param_1 + 8);
joined_r0x000180047002:
  do {
    while( true ) {
      while( true ) {
        if (plVar9 == plVar3) {
          return;
        }
        bVar2 = *(byte *)(plVar9 + 2);
        plVar4 = (longlong *)*plVar9;
        uVar10 = ((ulonglong)bVar2 ^ 0xcbf29ce484222325) * 0x100000001b3 &
                 *(ulonglong *)(param_1 + 0x30);
        puVar1 = (undefined8 *)(*(longlong *)(param_1 + 0x18) + uVar10 * 0x10);
        lVar11 = *(longlong *)(param_1 + 0x18) + uVar10 * 0x10;
        if ((longlong *)*puVar1 != plVar3) break;
        *puVar1 = plVar9;
        *(longlong **)(lVar11 + 8) = plVar9;
        plVar9 = plVar4;
      }
      plVar5 = *(longlong **)(lVar11 + 8);
      if (bVar2 != *(byte *)(plVar5 + 2)) break;
      plVar5 = (longlong *)*plVar5;
      if (plVar5 != plVar9) {
        plVar6 = (longlong *)plVar9[1];
        *plVar6 = (longlong)plVar4;
        puVar1 = (undefined8 *)plVar4[1];
        *puVar1 = plVar5;
        puVar7 = (undefined8 *)plVar5[1];
        *puVar7 = plVar9;
        plVar5[1] = (longlong)puVar1;
        plVar4[1] = (longlong)plVar6;
        plVar9[1] = (longlong)puVar7;
      }
      *(longlong **)(lVar11 + 8) = plVar9;
      plVar9 = plVar4;
    }
    do {
      if ((longlong *)*puVar1 == plVar5) {
        plVar6 = (longlong *)plVar9[1];
        *plVar6 = (longlong)plVar4;
        puVar7 = (undefined8 *)plVar4[1];
        *puVar7 = plVar5;
        puVar8 = (undefined8 *)plVar5[1];
        *puVar8 = plVar9;
        plVar5[1] = (longlong)puVar7;
        plVar4[1] = (longlong)plVar6;
        plVar9[1] = (longlong)puVar8;
        *puVar1 = plVar9;
        plVar9 = plVar4;
        goto joined_r0x000180047002;
      }
      plVar5 = (longlong *)plVar5[1];
    } while (bVar2 != *(byte *)(plVar5 + 2));
    lVar11 = *plVar5;
    plVar5 = (longlong *)plVar9[1];
    *plVar5 = (longlong)plVar4;
    plVar6 = (longlong *)plVar4[1];
    *plVar6 = lVar11;
    puVar1 = *(undefined8 **)(lVar11 + 8);
    *puVar1 = plVar9;
    *(longlong **)(lVar11 + 8) = plVar6;
    plVar4[1] = (longlong)plVar5;
    plVar9[1] = (longlong)puVar1;
    plVar9 = plVar4;
  } while( true );
}

