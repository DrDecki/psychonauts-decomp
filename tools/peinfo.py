#!/usr/bin/env python3
import struct
import sys
import time

VS_BUILDS = {
    8168: "VC++ 6.0",
    8447: "VC++ 6.0 SP5",
    8804: "VC++ 6.0 SP6",
    9466: "VS 2002 (7.0)",
    3077: "VS 2003 (7.1)",
    3052: "VS 2003 (7.1) free toolkit",
    4035: "VS 2003 (7.1) SDK",
    6030: "VS 2003 (7.1) SP1",
    50727: "VS 2005 (8.0)",
    50320: "VS 2005 (8.0) beta",
    21022: "VS 2008 (9.0)",
    30729: "VS 2008 (9.0) SP1",
    30319: "VS 2010 (10.0)",
    40219: "VS 2010 (10.0) SP1",
}


def rich_header(data, pe_off):
    end = data.find(b"Rich", 0x80, pe_off)
    if end < 0:
        return None
    key = struct.unpack_from("<I", data, end + 4)[0]
    entries = []
    pos = end - 8
    while pos >= 0x80:
        a, b = struct.unpack_from("<II", data, pos)
        a ^= key
        b ^= key
        if a == 0x536E6144:
            break
        entries.append((a >> 16, a & 0xFFFF, b))
        pos -= 8
    entries.reverse()
    return [e for e in entries if e != (0, 0, 0)]


def main():
    data = open(sys.argv[1], "rb").read()
    pe_off = struct.unpack_from("<I", data, 0x3C)[0]
    if data[pe_off:pe_off + 4] != b"PE\0\0":
        raise SystemExit("not a PE file")
    machine, nsec, stamp = struct.unpack_from("<HHI", data, pe_off + 4)
    opt = pe_off + 24
    magic = struct.unpack_from("<H", data, opt)[0]
    link_major, link_minor = data[opt + 2], data[opt + 3]
    code_size = struct.unpack_from("<I", data, opt + 4)[0]
    entry = struct.unpack_from("<I", data, opt + 16)[0]
    base = struct.unpack_from("<I", data, opt + 28)[0]
    print("machine 0x%x, optional header 0x%x" % (machine, magic))
    print("timestamp %s" % time.strftime("%Y-%m-%d %H:%M:%S UTC", time.gmtime(stamp)))
    print("linker %d.%02d, image base 0x%08x, entry 0x%08x, code size %d" % (link_major, link_minor, base, base + entry, code_size))
    opt_size = struct.unpack_from("<H", data, pe_off + 20)[0]
    sec = opt + opt_size
    print("sections:")
    for i in range(nsec):
        name = data[sec:sec + 8].rstrip(b"\0").decode("latin-1")
        vsize, vaddr, rsize, rptr = struct.unpack_from("<IIII", data, sec + 8)
        print("  %-8s va 0x%08x size %8d raw %8d" % (name, base + vaddr, vsize, rsize))
        sec += 40
    rich = rich_header(data, pe_off)
    if not rich:
        print("no rich header")
        return
    print("rich header (product id, build, count):")
    for prod, build, count in rich:
        print("  %3d  %5d  %5d  %s" % (prod, build, count, VS_BUILDS.get(build, "")))


if __name__ == "__main__":
    main()
