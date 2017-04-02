# NS-3 + OpenFlow

## Requisitos

 * Boost
 * Custom OSFID
   https://github.com/alexsanderdesouza/oflib

## Compilação do NS-3 com suporte a OpenFlow

    $ ./waf configure --with-openflow=../../openflow  --disable-python
    $ ./waf