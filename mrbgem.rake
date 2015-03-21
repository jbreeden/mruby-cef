MRuby::Gem::Specification.new('mruby-cef') do |spec|
  # NOTE:
  #  When building mruby on windows to link with the pre-built CEF via cl.exe,
  #  you may need to change the default /MD option of the visualstudio toolset to /MT.
  #  If not, you may see errors like "Some function foo was already defined in LIBCMTD.lib."
  #  Right now I'm accomplishing this with a block in my `build_config.rb`:
  #
  #  ```Ruby
  #  conf.cc.flags =  conf.cc.flags[0].map { |val|
  #    puts "Checking #{val} #{val == "/MD"}"
  #    val == "/MD" ? "/MT" : val
  #  }
  #
  #  conf.cxx.flags =  conf.cxx.flags[0].map { |val|
  #    puts "Checking #{val} #{val == "/MD"}"
  #    val == "/MD" ? "/MT" : val
  #  }
  #  ```
  #
  #  If you're building CEF from source, you can control your C runtime settings.
  #  Just make sure mruby, CEF, and you application all match.
  #  See: https://msdn.microsoft.com/en-us/library/2kzt1wy3.aspx

  spec.license = 'MIT'
  spec.author  = 'Jared Breeden'
  spec.summary = 'Bindings to the CEF libraries'
  spec.cxx.include_paths << "#{File.dirname(__FILE__)}/cef"
  spec.cc.include_paths << "#{File.dirname(__FILE__)}/cef"
end
