# avr-can-tool

- `avr-can-tool` is a simple tool to send and receive CAN messages 
- Based on ATmega328P (Arduino Pro Mini) and MCP2515
- Based on [AVRTOS](https://github.com/Adecy/AVRTOS) multithreading library
        - This project is particularly an example application of the AVRTOS multithreading library

What will be implemented
- Support for `caniot` commands in order to manage [caniot](https://github.com/Adecy/caniot-common) networks.

Example output :

```
> can tx AA 1 2 33 BB DD
> TX 00AA: 01 02 33 BB DD
RX 00AA: 02 02 33 BB DD

> 
> monitor threads
> ===== k_thread =====
R 0414 [PREE 1] WAITING : SP 48/100 -| END @0163
T 0426 [PREE 1] WAITING : SP 50/100 -| END @01C7
> 0438 [PREE 0] WAITING : SP 50/256 -| END @02C7
M 044A [PREE 0] READY   : SP 64669/100 -| END @058D
I 045C [PREE 2] READY   : SP 35/62 -| END @0305

> monitor canaries
> [R] CANARIES until @012C [found 44], MAX usage = 56 / 100
[T] CANARIES until @0194 [found 48], MAX usage = 52 / 100
[>] CANARIES until @0257 [found 143], MAX usage = 113 / 256
[M] CANARIES until @056A [found 64], MAX usage = 36 / 100
[I] CANARIES until @02D6 [found 14], MAX usage = 48 / 62
>
```

# Current threads max stack usage

```
[R] CANARIES until @0128 [found 40], MAX usage = 60 / 100
[T] CANARIES until @0183 [found 31], MAX usage = 69 / 100
[>] CANARIES until @025A [found 146], MAX usage = 110 / 256
[M] CANARIES until @0572 [found 64], MAX usage = 36 / 100
[I] CANARIES until @02D6 [found 14], MAX usage = 48 / 62
```

# Commands list

## `can` module

- Send can message using arbitration id `{id}` with data `{buffer}` using command :

```
can tx {id} {buffer}
```

Examples

```
can tx 12F AA BB CC DD EE FF 11 22
```

## `monitor` module

Get threads list :

```
monitor threads
```

Get threads canaries statistics :

```
monitor canaries
```

## `kernel` module

- Wait {delay} ms :

```
kernel wait 5000
```


----

- can
    - send
    - recv

    - set filter (loopback)
    - clear filter
    - reset
    - iface
    - stats (reset)
    - config
- caniot
- kernel
- monitor
        - threads
        - canaries

# Issues

- avr-can tool cannot receive EXTENDED ID can message. Maybe filters mask ? ...
        - maybe an explanation for the bug where not being able to receive a can message but able to send

# Updating from AVRTOS1 to AVRTOS2

## AVRTOS2

RX
```
[R] CANARIES until @0107 [found 6], MAX usage = 93 / 99 + 1 (sentinel)
[T] CANARIES until @018A [found 37], MAX usage = 62 / 99 + 1 (sentinel)
[C] CANARIES until @0243 [found 122], MAX usage = 37 / 159 + 1 (sentinel)
[>] CANARIES until @02F8 [found 143], MAX usage = 112 / 255 + 1 (sentinel)
[M] CANARIES until @06C8 [found 39], MAX usage = 40 / 79 + 1 (sentinel)
```

TX
```
[R] CANARIES until @0116 [found 21], MAX usage = 78 / 99 + 1 (sentinel)
[T] CANARIES until @0185 [found 32], MAX usage = 67 / 99 + 1 (sentinel)
[C] CANARIES until @0243 [found 122], MAX usage = 37 / 159 + 1 (sentinel)
[>] CANARIES until @02F8 [found 143], MAX usage = 112 / 255 + 1 (sentinel)
[M] CANARIES until @06C8 [found 39], MAX usage = 40 / 79 + 1 (sentinel)
```

## AVRTOS1