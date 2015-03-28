def configure_mruby_cef(conf)
  gem_dir = File.dirname(__FILE__)

  # Common include path (all platforms)
  conf.cc.include_paths << "#{gem_dir}/include"
  conf.cxx.include_paths << "#{gem_dir}/include"
  conf.cc.include_paths << "#{gem_dir}/include/cef"
  conf.cxx.include_paths << "#{gem_dir}/include/cef"

  if ENV['OS'] =~ /windows/i
    # Pre-built libraries are held under "#{gem_dir}/lib/PLATFORM/CONFIGURATION"
    conf.linker.library_paths << "#{gem_dir}/lib/win"

    # I've appended the selected C runtime libcef_dll_wrapper was built with onto the lib file name
    conf.linker.libraries << "libcef_dll_wrapper_md"
    conf.linker.libraries << "libcef"
    conf.linker.libraries << "User32"
  else
    raise "No mruby-cef build settings configured for this platform"
  end
end

MRuby::Gem::Specification.new('mruby-cef') do |spec|
  spec.license = 'MIT'
  spec.author  = 'Jared Breeden'
  spec.summary = 'Bindings to the CEF libraries'
  spec.bins = %w(lamina)
end
