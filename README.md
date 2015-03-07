mruby-cef
=========

Mruby bindings to CEF. A component of the [rb-chrome project](https://github.com/jbreeden/rb-chrome).

Notes for building
------------------

libcef is NOT included. If you try to build mruby with the default gemboxes, you will get 
unresolved linkage problems.

This is meant to build *only* libmruby, which will then need to be linked with a CEF 
application to work.

To get the mruby build to work with this gem, just ignore any gems in the default `mrbgems` directory
that have 'bin' in the name.

Notes for building on Windows
-----------------------------
When building mruby on windows to link with the pre-built CEF via cl.exe,
you may need to change the default /MD option of the visualstudio toolset to /MT.
If not, you may see errors like "Some function foo was already defined in LIBCMTD.lib."
Right now I'm accomplishing this with a block in my `build_config.rb`:

```Ruby
conf.cc.flags =  conf.cc.flags[0].map { |val|
  puts "Checking #{val} #{val == "/MD"}"
  val == "/MD" ? "/MT" : val
}

conf.cxx.flags =  conf.cxx.flags[0].map { |val|
  puts "Checking #{val} #{val == "/MD"}"
  val == "/MD" ? "/MT" : val
}
```

If you're building CEF from source, you can control your C runtime settings.
Just make sure mruby, CEF, and you application all match.
See: https://msdn.microsoft.com/en-us/library/2kzt1wy3.aspx
