$mruby_cef_gem_dir = File.expand_path(File.dirname(__FILE__))

def configure_mruby_cef_win(spec)
  spec.cc.include_paths << "#{$mruby_cef_gem_dir}/include/cef_win"
  spec.cxx.include_paths << "#{$mruby_cef_gem_dir}/include/cef_win"

  # Pre-built libraries are held under "#{gem_dir}/lib/PLATFORM"
  spec.linker.library_paths << "#{$mruby_cef_gem_dir}/lib/win"

  # I've appended the selected C runtime libcef_dll_wrapper was built with onto the lib file name
  spec.linker.libraries << "libcef_dll_wrapper_md"
  spec.linker.libraries << "libcef"
  spec.linker.libraries << "User32"
end

def configure_mruby_cef_lin64(spec)
  spec.cc.include_paths << "#{$mruby_cef_gem_dir}/include/cef_lin"
  spec.cxx.include_paths << "#{$mruby_cef_gem_dir}/include/cef_lin"

  ENV['LD_LIBRARY_PATH'] = "#{$mruby_cef_gem_dir}/lib/lin64:" + (ENV['LD_LIBRARY_PATH'] || '')
  spec.linker.library_paths << "#{$mruby_cef_gem_dir}/lib/lin64"
  spec.linker.flags_before_libraries << "#{$mruby_cef_gem_dir}/lib/lin64/libcef_dll_wrapper.a"
  spec.linker.libraries << 'cef'
end

def configure_mruby_cef_mac(spec)
  spec.linker.flags << "-F/Users/jared/projects/cef-build/cef_binary_3.2171.1979_macosx64/Release"
  spec.linker.flags << "-framework \"Chromium Embedded Framework\""
end

def configure_mruby_cef(spec)
  # Common include path (all platforms)
  spec.cc.include_paths << "#{$mruby_cef_gem_dir}/include"
  spec.cxx.include_paths << "#{$mruby_cef_gem_dir}/include"

  # TODO
  spec.cc.include_paths << "/Users/jared/projects/cef-build/cef_binary_3.2171.1979_macosx64"
  spec.cxx.include_paths << "/Users/jared/projects/cef-build/cef_binary_3.2171.1979_macosx64"

  if OS.windows?
    configure_mruby_cef_win(spec)
  elsif OS.mac?

  elsif `uname -a` =~ /x86_64/
    configure_mruby_cef_lin64(spec)
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
  configure_mruby_cef(spec)
end
