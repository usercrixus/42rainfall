#!/bin/bash

qemu-system-x86_64 \
  -machine q35 \
  -smp 4 \
  -m 2G \
  -cdrom ./RainFall.iso \
  -netdev user,id=vmnic,hostname=lfshost,hostfwd=tcp::4242-:4242,hostfwd=tcp::4444-:4444 \
  -device virtio-net,netdev=vmnic \
  -device virtio-rng-pci &

