"""Optional Sedona modules for the ESP32 build.

Each module adds kits to kits.scode, native kits and native sources to the
platform, host libraries (simulator) and a piece of the demo app
(apps/modules/<name>.xml).  'targets' says where it builds today:
  sim    the PC simulator (platform/sim)
  esp32  the ESP-IDF firmware

Select with  build.py ... --modules all | default | web,mqtt,...
  default = every module supported by the target
"""

MODULES = {
    'web': dict(
        doc='HTTP server (web::WebService), tcp/8080',
        kits=['web'], targets={'sim', 'esp32'}),
    'pstore': dict(
        doc='persistent storage service',
        kits=['pstore'], targets={'sim', 'esp32'}),
    'logmanager': dict(
        doc='log level management',
        kits=['logManager'], targets={'sim', 'esp32'}),
    'inout': dict(
        doc='generic Bclimate I/O points (bclimateInOutGeneric)',
        kits=['bclimateInOutGeneric'], targets={'sim', 'esp32'}),
    'libnative': dict(
        doc='Bclimate native library: conversions, license, network config',
        kits=['bclimateLibNative'], native_kits=['bclimateLibNative'],
        sources=['/src/bclimateLibNative/native'], targets={'sim', 'esp32'}),
    'expression': dict(
        doc='formula blocks (bclimateExpression::ExpressionFloat/Bool)',
        kits=['bclimateExpression'], native_kits=['bclimateExpression'],
        sources=['/src/bclimateExpression/native'], targets={'sim', 'esp32'}),
    'uiconnect': dict(
        doc='Bclimate UI connection service, tcp/8088',
        kits=['bclimateUIConnect'], requires=['libnative'], targets={'sim', 'esp32'}),
    'mqtt': dict(
        doc='MQTT client (communityMQTT): publish/subscribe points',
        kits=['communityMQTT'], native_kits=['communityMQTT'],
        sources=['/src/communityMQTT/native'],
        # firmware: the worker is woken with pthread_kill(SIGUSR1); ESP-IDF has
        # no POSIX signals, so it needs a poll/queue wake-up first
        targets={'sim'}),
    'modbus': dict(
        doc='Modbus TCP/RTU master and slave (libmodbus), slave on tcp/1502',
        kits=['serial', 'bclimateModbusMaster', 'bclimateModbusSlave'],
        native_kits=['serial', 'bclimateModbusMaster', 'bclimateModbusSlave'],
        sources=['/src/serial/native', '/src/serial/native/linux',
                 '/src/bclimateModbusMaster/native', '/src/bclimateModbusSlave/native'],
        libs=['-lmodbus'], cflags=['-I/usr/include/modbus'],
        requires=['libnative'],
        # firmware: needs libmodbus as an ESP-IDF component and a UART serial port
        targets={'sim'}),
}


def select(spec, target):
    """Module names for spec ('default', 'all', 'none' or a comma list) on target."""
    if spec in (None, '', 'default'):
        names = [n for n, m in MODULES.items() if target in m['targets']]
    elif spec == 'all':
        names = list(MODULES)
    elif spec == 'none':
        names = []
    else:
        names = [n.strip() for n in spec.split(',') if n.strip()]
    unknown = [n for n in names if n not in MODULES]
    if unknown:
        raise SystemExit('unknown module(s): %s; known: %s' % (', '.join(unknown), ', '.join(MODULES)))
    out = []

    def add(n):
        if n in out:
            return
        for r in MODULES[n].get('requires', []):
            add(r)
        out.append(n)
    for n in names:
        add(n)
    bad = [n for n in out if target not in MODULES[n]['targets']]
    if bad:
        raise SystemExit('module(s) not available for %s yet: %s' % (target, ', '.join(bad)))
    return out


def describe():
    return '\n'.join('  %-11s %-10s %s' % (n, '+'.join(sorted(m['targets'])), m['doc'])
                     for n, m in MODULES.items())
