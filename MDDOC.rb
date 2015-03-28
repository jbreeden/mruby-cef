def doc_start(filename)
  case filename
  when /c(pp)?$/
    '//<'
  when /rb$/
    '#<'
  else
    nil
  end
end

def doc_prefix(filename)
  case filename
  when /c(pp)?$/
    '//'
  when /rb$/
    '#'
  else
    nil
  end
end

def doc_end(filename)
  case filename
  when /c(pp)?$/
    '//>'
  when /rb$/
    '#>'
  else
    nil
  end
end

def files
  ["./src/mruby_cef_v8.cpp"]
end
