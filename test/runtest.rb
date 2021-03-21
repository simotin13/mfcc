require 'open3'
require 'fileutils'
require 'color_echo'
bindir = "../bin"
cc = "gpcc"

test_specs = [
  {src: "return_integer.c"}
]
# build gpcc
pwd = Dir.pwd
Dir.chdir("../src")
stdout, stderr, status = Open3.capture3("make")
unless status.exitstatus == 0
  # build gpcc failed
  puts "build gpcc failed #{stderr}"
  exit status.exitstatus
end
FileUtils.cp("gpcc", bindir)

Dir.chdir(pwd)
sources = Dir.glob('**/*.c')
sources.each do |source|
  cmd = "#{bindir}/#{cc} #{source}"
  puts cmd
  stdout, stderr, status = Open3.capture3(cmd)
  unless status.exitstatus == 0
    # compile test source failed
    CE.fg(:red)
    puts "#{cmd} failed"
    next
  end
  asemble_cmd = "nasm -f elf64 tmp.s"
  stdout, stderr, status = Open3.capture3(asemble_cmd)
  link_cmd = "gcc tmp.o"
  stdout, stderr, status = Open3.capture3(link_cmd)
  exec_cmd = `./a.out`
  stdout, stderr, status = Open3.capture3(exec_cmd)
  test_spec_method = File.basename(source, ".c")

  puts stdout
  puts stderr
  puts status.exitstatus
end
