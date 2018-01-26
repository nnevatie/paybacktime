# Create skeleton
skeleton = "#{ARGV[1]}/skeleton.ozz"
system("fbx2skel --file=#{ARGV[0]}/pose.fbx --skeleton=#{skeleton}")

# Create animations archive
archive = "#{ARGV[1]}/archive.ozz"
File.open(archive, 'wb') do |output|
  Dir.foreach(ARGV[0]) do |file|
    next if file == '.' or file == '..'
    animation = "#{ARGV[1]}/#{File.basename(file, ".fbx")}.ozz"
    system("fbx2anim --log_level=verbose --file=#{ARGV[0]}/#{file} --skeleton=#{skeleton} " \
           "--animation=#{animation}")
    #output.write(File.open(animation, 'rb').read)
  end
end
