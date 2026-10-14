# storage/hal -- driver key table

`hal.h` and `hal_table.c` list, for every storage IP we know, the key Linux
uses to bind its driver -- a DTS `compatible` string read from that driver's
`of_match_table`, or a PCI class code where that is how the kernel binds (NVMe).
Each row carries the four-level position (class / subclass / IP vendor / IP
model) and the kernel source line the key was read from.

Ask for an IP by name and you get either the model that drives it or an
explicit "not implemented" -- never a silent fallback. Unknown name ->
`HAL_ENODEV`; known but not linked in this build -> `HAL_ENOSYS`.
