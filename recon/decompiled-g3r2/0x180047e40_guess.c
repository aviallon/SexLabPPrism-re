// PrismExport addr=0x180047e40 name=guess ghidra=FUN_180047e40 status=clean lines=145

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

undefined8 * FUN_180047e40(undefined8 *param_1,undefined1 (*param_2) [16])

{
  char cVar1;
  code *pcVar2;
  void *pvVar3;
  void *pvVar4;
  void *pvVar5;
  bool bVar6;
  bool bVar7;
  undefined1 (*pauVar8) [16];
  size_t sVar9;
  longlong lVar10;
  void **ppvVar11;
  void *pvVar12;
  undefined1 (*pauVar13) [16];
  byte *pbVar14;
  undefined1 (*pauVar15) [16];
  undefined1 *puVar16;
  longlong lVar17;
  undefined1 auStack_1b8 [8];
  undefined1 auStack_1b0 [24];
  undefined4 local_198;
  void *local_190;
  undefined8 uStack_188;
  undefined8 local_180;
  ulonglong local_178;
  undefined8 *local_170;
  char local_168 [256];
  void *local_68;
  undefined8 uStack_60;
  undefined8 local_58;
  ulonglong uStack_50;
  ulonglong local_48;
  
  local_48 = DAT_180095900 ^ (ulonglong)auStack_1b8;
  local_198 = 0;
  local_170 = param_1;
  sVar9 = strlen("\\/");
  pauVar15 = param_2;
  if (0xf < *(ulonglong *)(param_2[1] + 8)) {
    pauVar15 = *(undefined1 (**) [16])*param_2;
  }
  if ((sVar9 == 0) || (*(longlong *)param_2[1] == 0)) {
    puVar16 = &DAT_ffffffffffffffff;
  }
  else {
    lVar10 = *(longlong *)param_2[1] + -1;
    puVar16 = &DAT_ffffffffffffffff;
    lVar17 = -1;
    if (lVar10 != -1) {
      lVar17 = lVar10;
    }
    if ((undefined1 *)(lVar17 + 1) + sVar9 < (undefined1 *)0x10) {
      memset(local_168,0,0x100);
      for (pbVar14 = &DAT_180061a4c; &DAT_180061a4c + sVar9 != pbVar14; pbVar14 = pbVar14 + 1) {
        local_168[*pbVar14] = '\x01';
      }
      pauVar13 = (undefined1 (*) [16])(*pauVar15 + lVar17);
      cVar1 = local_168[(byte)(*pauVar15)[lVar17]];
      while (cVar1 == '\0') {
        if (pauVar13 == pauVar15) goto LAB_180047f62;
        pauVar8 = pauVar13 + -1;
        pauVar13 = (undefined1 (*) [16])(pauVar13[-1] + 0xf);
        cVar1 = local_168[(byte)(*pauVar8)[0xf]];
      }
      puVar16 = (undefined1 *)((longlong)pauVar13 - (longlong)pauVar15);
    }
    else {
      puVar16 = thunk_FUN_18004ec20(pauVar15,(undefined1 *)(lVar17 + 1),
                                    (undefined1 (*) [16])&DAT_180061a4c,sVar9);
    }
  }
LAB_180047f62:
  if (puVar16 == &DAT_ffffffffffffffff) {
    uStack_188 = 0;
    local_190 = (void *)0x0;
    ppvVar11 = &local_190;
    local_180 = 0;
    bVar7 = false;
    bVar6 = true;
    local_178 = 0xf;
  }
  else {
    local_68 = (void *)0x0;
    uStack_60 = 0;
    if (*(undefined1 **)param_2[1] < puVar16) {
      puVar16 = *(undefined1 **)param_2[1];
    }
    local_58 = 0;
    uStack_50 = 0;
    if (0xf < *(ulonglong *)(param_2[1] + 8)) {
      param_2 = *(undefined1 (**) [16])*param_2;
    }
    FUN_180001670(&local_68,param_2,(size_t)puVar16);
    ppvVar11 = &local_68;
    bVar7 = true;
    bVar6 = false;
  }
  pvVar12 = *ppvVar11;
  pvVar3 = ppvVar11[1];
  pvVar4 = ppvVar11[2];
  pvVar5 = ppvVar11[3];
  ppvVar11[2] = (void *)0x0;
  *(undefined1 *)ppvVar11 = 0;
  ppvVar11[3] = (void *)0xf;
  *param_1 = pvVar12;
  param_1[1] = pvVar3;
  param_1[2] = pvVar4;
  param_1[3] = pvVar5;
  if ((bVar6) && (0xf < local_178)) {
    pvVar12 = local_190;
    if ((local_178 + 1 < 0x1000) ||
       (pvVar12 = *(void **)((longlong)local_190 + -8),
       (ulonglong)((longlong)local_190 + (-8 - (longlong)pvVar12)) < 0x20)) {
      free(pvVar12);
      goto LAB_18004806b;
    }
LAB_1800480a9:
    pcVar2 = (code *)swi(0x29);
    pvVar12 = (void *)(*pcVar2)(5);
    puVar16 = auStack_1b0;
  }
  else {
LAB_18004806b:
    puVar16 = auStack_1b8;
    if ((!bVar7) || (puVar16 = auStack_1b8, uStack_50 < 0x10)) goto LAB_1800480b8;
    pvVar12 = local_68;
    puVar16 = auStack_1b8;
    if ((0xfff < uStack_50 + 1) &&
       (pvVar12 = *(void **)((longlong)local_68 + -8), puVar16 = auStack_1b8,
       0x1f < (ulonglong)((longlong)local_68 + (-8 - (longlong)*(void **)((longlong)local_68 + -8)))
       )) goto LAB_1800480a9;
  }
  *(undefined8 *)(puVar16 + -8) = 0x1800480b8;
  free(pvVar12);
LAB_1800480b8:
  *(undefined8 *)(puVar16 + -8) = 0x1800480cb;
  return param_1;
}

