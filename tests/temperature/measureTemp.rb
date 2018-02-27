abort "Not enough input" if ARGV.size<2
interval=ARGV[0]
folderOut=ARGV[1]+"/temperatures/"
puts "> Measuring temperature with interval: #{interval} sec\noutput in folder: #{folderOut}..."
system("mkdir -p #{folderOut}")
while(true)
	time=Time.now.to_i
	system("sensors > #{folderOut}/temperature_#{time}.log")	
	# Sleeping for X seconds
	sleep interval.to_i
end
