# Mruby-pax

### 1.3.1 - 2016-10-28
- Fixed Magnetic.tracks returns, sometimes it was returning empty.

### 1.3.0 - 2016-10-06

- Implement custom back and forward keys.
- define default values for scripts variables.
- Add PAX::EMV.param_flag.
- Bug fix properly send cashback in EMVStartTrans.
- Refactoring SetMCKParam to get params before set.
- Remove function logContext, replaced by ContextLog.
- Touch.c
- Rename ContextLog.error by ContextLog.exception
- Implement Touch bind.
- Only open MSR once.
- Remove display at Emv.del_pkis.
- Refactoring cEMVWaitAppSel to send a unique hash instead of hash and arrya(labels) to callback.
- Bug fix, check if cash back is 0 to send NULL and avoid Application Usage Control processing problems.
- Always set ucBypassPin 0 and ucBatchCapture 1.
- Review table parameters values and sizes at emv.c
- Perform PKI check sum calculation if statu_check_sum is 0.
- Review all commentaries and values at emv.rb.
- Store pkis in Emv.pkis instead of load in initialisation time, and let to dynamically load it in transaction time.
- Implement EMV.random.
- Refactoring returns and commentaries at src/emv.c.
- Add logContext function to call ContextLog.info in C.
- Fix rss object consumption in Pinpad operations.
- Refactoring OsPedSetAsteriskLayout call to wait input in the right line.
- Add EMV.del_pkis.
- Return EMV_OK in cEMVSetParam and cEMVSM3.
- Add function to log emv errors.
- Refactoring pki parse calculating pki check sum.
