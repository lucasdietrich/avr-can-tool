# AVRTOS 1/2 comparison

##AVRTOS 1 : RX

```
RAM:   [=======   ]  72.6% (used 1486 bytes from 2048 bytes)
Flash: [=======   ]  70.7% (used 21732 bytes from 30720 bytes)
===== k_thread =====
R 0574 [COOP 1] READY   : SP 34/100 -| END @0163
T 0586 [COOP 1] READY   : SP 34/100 -| END @01C7
C 0598 [COOP 1] READY   : SP 34/160 -| END @0267
> 05AA [PREE 1] READY   : SP 34/256 -| END @0367
M 05BC [PREE 1] READY   : SP 0/80 -| END @06BA

[R] CANARIES until @010F [found 14], MAX usage = 85 / 99 + 1 (sentinel)
[T] CANARIES until @018C [found 39], MAX usage = 60 / 99 + 1 (sentinel)
[C] CANARIES until @0227 [found 94], MAX usage = 65 / 159 + 1 (sentinel)
[>] CANARIES until @02DA [found 113], MAX usage = 142 / 255 + 1 (sentinel)
[M] CANARIES until @0696 [found 42], MAX usage = 37 / 79 + 1 (sentinel)
```

## AVRTOS 1 : TX

```
RAM:   [=======   ]  72.6% (used 1486 bytes from 2048 bytes)
Flash: [=======   ]  70.7% (used 21732 bytes from 30720 bytes)
R 0574 [COOP 1] READY   : SP 34/100 -| END @0163
T 0586 [COOP 1] READY   : SP 34/100 -| END @01C7
C 0598 [COOP 1] READY   : SP 34/160 -| END @0267
> 05AA [PREE 1] READY   : SP 34/256 -| END @0367
M 05BC [PREE 1] READY   : SP 0/80 -| END @06BA

[R] CANARIES until @0118 [found 23], MAX usage = 76 / 99 + 1 (sentinel) 
[T] CANARIES until @018C [found 39], MAX usage = 60 / 99 + 1 (sentinel) 
[C] CANARIES until @0227 [found 94], MAX usage = 65 / 159 + 1 (sentinel)
[>] CANARIES until @02FA [found 145], MAX usage = 110 / 255 + 1 (sentinel)
[M] CANARIES until @0696 [found 42], MAX usage = 37 / 79 + 1 (sentinel)
```

---

## AVRTOS 2 : RX

```
RAM:   [========  ]  75.1% (used 1539 bytes from 2048 bytes)
Flash: [=======   ]  71.9% (used 22090 bytes from 30720 bytes)

R 0x057E READY   C ____ : SP 21/100:0x0163
T 0x0592 READY   C ____ : SP 21/100:0x01C7
C 0x05A6 READY   C ____ : SP 21/160:0x0267
> 0x05BA READY   P ____ : SP 21/256:0x0367
M 0x05CE READY   P ____ : SP 0/80:0x06EF

[R] CANARIES until @0116 [found 21], MAX usage = 78 / 99 + 1 (sentinel)   
[T] CANARIES until @018A [found 37], MAX usage = 62 / 99 + 1 (sentinel)   
[C] CANARIES until @0243 [found 122], MAX usage = 37 / 159 + 1 (sentinel) 
[>] CANARIES until @02F8 [found 143], MAX usage = 112 / 255 + 1 (sentinel)
[M] CANARIES until @06C8 [found 39], MAX usage = 40 / 79 + 1 (sentinel)   
```

## AVRTOS 2 : TX

```
RAM:   [========  ]  75.1% (used 1539 bytes from 2048 bytes)
Flash: [=======   ]  71.9% (used 22090 bytes from 30720 bytes)

R 0x057E READY   C ____ : SP 21/100:0x0163
T 0x0592 READY   C ____ : SP 21/100:0x01C7
C 0x05A6 READY   C ____ : SP 21/160:0x0267
> 0x05BA READY   P ____ : SP 21/256:0x0367
M 0x05CE READY   P ____ : SP 0/80:0x06EF

[R] CANARIES until @0116 [found 21], MAX usage = 78 / 99 + 1 (sentinel)   
[T] CANARIES until @018A [found 37], MAX usage = 62 / 99 + 1 (sentinel)   
[C] CANARIES until @0243 [found 122], MAX usage = 37 / 159 + 1 (sentinel) 
[>] CANARIES until @02F8 [found 143], MAX usage = 112 / 255 + 1 (sentinel)
[M] CANARIES until @06C8 [found 39], MAX usage = 40 / 79 + 1 (sentinel)
```