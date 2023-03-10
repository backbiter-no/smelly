smelly.conf
-----------------------

.. highlight:: conf

|smelly| is highly customizable, everything from keyboard shortcuts, to rendering
frames-per-second. See below for an overview of all customization possibilities.

You can open the config file within smelly by pressing :sc:`edit_config_file`
(:kbd:`⌘+,` on macOS). A :file:`smelly.conf` with commented default
configurations and descriptions will be created if the file does not exist.
You can reload the config file within smelly by pressing :sc:`reload_config_file`
(:kbd:`⌃+⌘+,` on macOS) or sending smelly the ``SIGUSR1`` signal.
You can also display the current configuration by pressing :sc:`debug_config`
(:kbd:`⌥+⌘+,` on macOS).

.. _confloc:

|smelly| looks for a config file in the OS config directories (usually
:file:`~/.config/smelly/smelly.conf`) but you can pass a specific path via the
:option:`smelly --config` option or use the :envvar:`smelly_CONFIG_DIRECTORY`
environment variable. See :option:`smelly --config` for full details.

Comments can be added to the config file as lines starting with the ``#``
character. This works only if the ``#`` character is the first character in the
line.

.. _include:

You can include secondary config files via the :code:`include` directive. If
you use a relative path for :code:`include`, it is resolved with respect to the
location of the current config file. Note that environment variables are
expanded, so :code:`${USER}.conf` becomes :file:`name.conf` if
:code:`USER=name`. A special environment variable :envvar:`smelly_OS` is available,
to detect the operating system. It is ``linux``, ``macos`` or ``bsd``.
Also, you can use :code:`globinclude` to include files
matching a shell glob pattern and :code:`envinclude` to include configuration
from environment variables. For example::

     include other.conf
     # Include *.conf files from all subdirs of smelly.d inside the smelly config dir
     globinclude smelly.d/**/*.conf
     # Include the *contents* of all env vars starting with smelly_CONF_
     envinclude smelly_CONF_*


.. note:: Syntax highlighting for :file:`smelly.conf` in vim is available via
   `vim-smelly <https://github.com/fladson/vim-smelly>`__.


.. include:: /generated/conf-smelly.rst


Sample smelly.conf
--------------------

.. only:: html

    You can download a sample :file:`smelly.conf` file with all default settings
    and comments describing each setting by clicking: :download:`sample
    smelly.conf </generated/conf/smelly.conf>`.

.. only:: man

   You can edit a fully commented sample smelly.conf by pressing the
   :sc:`edit_config_file` shortcut in smelly. This will generate a config file
   with full documentation and all settings commented out. If you have a
   pre-existing :file:`smelly.conf`, then that will be used instead, delete it to
   see the sample file.

A default configuration file can also be generated by running::

    smelly +runpy 'from smelly.config import *; print(commented_out_default_config())'

This will print the commented out default config file to :file:`STDOUT`.

All mappable actions
------------------------

See the :doc:`list of all the things you can make smelly can do </actions>`.

.. toctree::
   :hidden:

   actions
