#!/usr/bin/env python
# vim:fileencoding=utf-8
# License: GPLv3 Copyright: 2020, Anders Pippi Tednes <theblckswan@protonmail.com>

import io
import os
import shlex
import shutil
import subprocess
import sys
import tarfile
from urllib.request import urlopen

is_bundle = os.environ.get('smelly_BUNDLE') == '1'
is_macos = 'darwin' in sys.platform.lower()
SW = None


def run(*a):
    if len(a) == 1:
        a = shlex.split(a[0])
    print(' '.join(map(shlex.quote, a)))
    sys.stdout.flush()
    ret = subprocess.Popen(a).wait()
    if ret != 0:
        raise SystemExit(ret)


def install_deps():
    print('Installing smelly dependencies...')
    sys.stdout.flush()
    if is_macos:
        items = [x.split()[1].strip('"') for x in open('Brewfile').readlines() if x.strip().startswith('brew ')]
        openssl = 'openssl'
        items.remove('go')  # already installed by ci.yml
        import ssl
        if ssl.OPENSSL_VERSION_INFO[0] == 1:
            openssl += '@1.1'
        run('brew', 'install', 'fish', openssl, *items)
    else:
        run('sudo apt-get update')
        run('sudo apt-get install -y libgl1-mesa-dev libxi-dev libxrandr-dev libxinerama-dev ca-certificates'
            ' libxcursor-dev libxcb-xkb-dev libdbus-1-dev libxkbcommon-dev libharfbuzz-dev libx11-xcb-dev zsh'
            ' libpng-dev liblcms2-dev libfontconfig-dev libxkbcommon-x11-dev libcanberra-dev librsync-dev uuid-dev'
            ' zsh bash dash')
        # for some reason these directories are world writable which causes zsh
        # compinit to break
        run('sudo chmod -R og-w /usr/share/zsh')
    if is_bundle:
        install_bundle()
    else:
        cmd = 'python3 -m pip install Pillow pygments'
        if sys.version_info[:2] < (3, 7):
            cmd += ' importlib-resources dataclasses'
        run(cmd)


def build_smelly():
    python = shutil.which('python3') if is_bundle else sys.executable
    cmd = f'{python} setup.py build --verbose'
    if os.environ.get('smelly_SANITIZE') == '1':
        cmd += ' --debug --sanitize'
    run(cmd)


def test_smelly():
    run('./test.py')


def package_smelly():
    python = 'python3' if is_macos else 'python'
    run(f'{python} setup.py linux-package --update-check-interval=0 --verbose')
    if is_macos:
        run('python3 setup.py smelly.app --update-check-interval=0 --verbose')
        run('smelly.app/Contents/MacOS/smelly +runpy "from smelly.constants import *; print(smelly_exe())"')


def replace_in_file(path, src, dest):
    with open(path, 'r+') as f:
        n = f.read().replace(src, dest)
        f.seek(0), f.truncate()
        f.write(n)


def setup_bundle_env():
    global SW
    os.environ['SW'] = SW = '/Users/Shared/smelly-build/sw/sw' if is_macos else os.path.join(os.environ['GITHUB_WORKSPACE'], 'sw')
    os.environ['PKG_CONFIG_PATH'] = os.path.join(SW, 'lib', 'pkgconfig')
    if is_macos:
        os.environ['PATH'] = '{}:{}'.format('/usr/local/opt/sphinx-doc/bin', os.environ['PATH'])
    else:
        os.environ['LD_LIBRARY_PATH'] = os.path.join(SW, 'lib')
        os.environ['PYTHONHOME'] = SW
    os.environ['PATH'] = '{}:{}'.format(os.path.join(SW, 'bin'), os.environ['PATH'])


def install_bundle():
    cwd = os.getcwd()
    os.makedirs(SW)
    os.chdir(SW)
    with urlopen('https://download.calibre-ebook.com/ci/smelly/{}-64.tar.xz'.format(
            'macos' if is_macos else 'linux')) as f:
        data = f.read()
    with tarfile.open(fileobj=io.BytesIO(data), mode='r:xz') as tf:
        tf.extractall()
    if not is_macos:
        replaced = 0
        for dirpath, dirnames, filenames in os.walk('.'):
            for f in filenames:
                if f.endswith('.pc') or (f.endswith('.py') and f.startswith('_sysconfig')):
                    replace_in_file(os.path.join(dirpath, f), '/sw/sw', SW)
                    replaced += 1
        if replaced < 2:
            raise SystemExit('Failed to replace path to SW in bundle')
    os.chdir(cwd)


def main():
    if is_bundle:
        setup_bundle_env()
    else:
        if not is_macos and 'pythonLocation' in os.environ:
            os.environ['LD_LIBRARY_PATH'] = os.path.join(os.environ['pythonLocation'], 'lib')
    action = sys.argv[-1]
    if action in ('build', 'package'):
        install_deps()
    if action == 'build':
        build_smelly()
    elif action == 'package':
        package_smelly()
    elif action == 'test':
        test_smelly()
    elif action == 'gofmt':
        q = subprocess.check_output('gofmt -s -l tools'.split())
        if q.strip():
            q = '\n'.join(filter(lambda x: not x.rstrip().endswith('_generated.go'), q.decode().strip().splitlines())).strip()
            if q:
                raise SystemExit(q)
    else:
        raise SystemExit(f'Unknown action: {action}')


if __name__ == '__main__':
    main()
