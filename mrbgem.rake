$mruby_cef_gem_dir = File.expand_path(File.dirname(__FILE__))

def configure_mruby_cef_win(conf)
  conf.cc.include_paths << "#{$mruby_cef_gem_dir}/include/cef_win"
  conf.cxx.include_paths << "#{$mruby_cef_gem_dir}/include/cef_win"
  
  # Pre-built libraries are held under "#{gem_dir}/lib/PLATFORM"
  conf.linker.library_paths << "#{$mruby_cef_gem_dir}/lib/win"

  # I've appended the selected C runtime libcef_dll_wrapper was built with onto the lib file name
  conf.linker.libraries << "libcef_dll_wrapper_md"
  conf.linker.libraries << "libcef"
  conf.linker.libraries << "User32"
end

def configure_mruby_cef_lin64(conf)
  conf.cc.include_paths << "#{$mruby_cef_gem_dir}/include/cef_lin"
  conf.cxx.include_paths << "#{$mruby_cef_gem_dir}/include/cef_lin"
  
  ENV['LD_LIBRARY_PATH'] = "#{$mruby_cef_gem_dir}/lib/lin64:" + (ENV['LD_LIBRARY_PATH'] || '')
  conf.linker.library_paths << "#{$mruby_cef_gem_dir}/lib/lin64"
  conf.linker.flags_before_libraries << "#{$mruby_cef_gem_dir}/lib/lin64/libcef_dll_wrapper.a"
  conf.linker.libraries << 'cef'
end

def configure_mruby_cef(conf)
  # Common include path (all platforms)
  conf.cc.include_paths << "#{$mruby_cef_gem_dir}/include"
  conf.cxx.include_paths << "#{$mruby_cef_gem_dir}/include"

  if ENV['OS'] =~ /windows/i
    configure_mruby_cef_win(conf)
  elsif `uname -a` =~ /x86_64/
    configure_mruby_cef_lin64(conf)
  else
    raise 'No mruby-cef build configuration for this platform'
  end
end

MRuby::Gem::Specification.new('mruby-cef') do |spec|
  spec.license = 'MIT'
  spec.author  = 'Jared Breeden'
  spec.summary = 'Bindings to the CEF libraries'
  
  spec.cc.flags << [ '-std=c11' ]
  spec.cxx.flags << [ '-std=c++11' ]
end
