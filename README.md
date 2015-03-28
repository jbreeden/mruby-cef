mruby-cef
=========

Mruby bindings to CEF. A component of the [lamina project](https://github.com/jbreeden/lamina).

Check out the [examples page](http://jbreeden.github.io/mruby-cef/) for details on interacting with JavaScript using mruby-cef. The examples page was extracted from the mruby-extensions sample in [lamina](https://github.com/jbreeden/lamina).

Building
--------

To build this gem into your mruby instance, just download the repo and add a couple of lines to your `build_config.rb` file:

```Ruby
MRuby::Build.new('host') do |conf|
  # ... Other configurations...

  # Include the mruby-apr gem
  conf.gem "PATH/TO/mruby-cef"

  # Call configure_muby_apr with the `conf` object
  # (This just sets the required compiler options)
  configure_mruby_cef(conf)

end
```

Note: The `configure_mruby_cef` function currently only supports the Windows platform at the moment. If you're on another system, have a quick peak at the `mrbgem.rake` file and replace the body of the `else` clause in the platform check - shown below - with the required setup for your platform (pull requests are welcomed ;).

```
if ENV['OS'] =~ /windows/i
  # ... already implemented ....
else
  raise "No mruby-cef build settings configured for this platform"
end
```
