require 'optparse'

def cpuMemTest(resFile)
	pid=`ps aux | grep "chrome --type=renderer" | awk 'FNR>1 {print $2}'`.split("\n").first
	puts "\n> Measuring CPU and MEM in pid: #{pid}..."
	system("psrecord --include-children --interval 1 --plot #{resFile}_memCPU.png --log #{resFile}_memCPU.log #{pid} &")
end

def interference(domain,resFile,t)
	puts "> Probe (No2) with inteference..."
	system("google-chrome --incognito --disable-extensions http://#{domain} > /dev/null 2>&1 &")
	system("./tests/interference/y-cruncher-v0.7.5.9480-static/y-cruncher custom pi -dec:1b > #{resFile}_interference.log &")
	sleep(t)
	system("kill -9 $(pgrep 11-SNB)")
	system("kill -9 $(pgrep chrome)")
end

def temperature(resFile)
	system("ruby ./tests/temperature/measureTemp.rb 1 #{resFile} &")
end

def power(resFile)
	puts "> Measuring power..."
	system("./tests/power/power > #{resFile}_power.csv &")
end

def getHar(resFile,domain)
	system("chrome-har-capturer -c -o #{resFile}_requests.har http://#{domain} &")
end

#parameters
options = {}
list=nil
dir="probeResults/"
time=1800 #1800sec = 30min
OptionParser.new do |opts|
  opts.banner = "Usage: #{__FILE__} [options] <SiteList>"
  opts.on("-t", "--time [TIME]", "Time (sec) to wait for each probe [default:30 min]") do |v|
    time = v.to_i
  end
  opts.on_tail("-h", "--help", "Show help message") do
    puts opts
    exit
  end
end.parse!
abort "Error: No list for input!" if ARGV.size<1
list=ARGV[0]

#read domainlist
doms=Array.new
print "Reading #{list}..."
File.foreach(list){|line| doms.push(line.split("\t").first)}
puts "done!\nStarting with the probes..."
system("mkdir -p #{dir}")
thereIsPhidget=false
puts "Warning: No sensor found. Proceeding without measuring power..." if not `if [ -f tests/power/power ]; then echo "exists" ; fi;`.include? "exists"

#start mining probes
doms.each{|domain|
domain="beasiswamext.or.id"
	puts "Probing "+domain
	filename=domain.gsub("/","-")
	headDir=dir+filename
	resFile=headDir+"/"+filename
	system("mkdir -p #{headDir}/memCPU/")
	puts "> Opening Chrome..."
	system("google-chrome --incognito --headless --disable-extensions --remote-debugging-port=9222 > /dev/null 2>&1 &")
	sleep(2)

	#run Tests
	power(resFile) if thereIsPhidget	    # (1) system power 
	temperature(filename)   # (2) system temperature
	getHar(resFile,domain) 					# (3) get har
	sleep(1)
	cpuMemTest("#{headDir}/memCPU/"+filename) 					# (4) cpu & mem

	sleep(time)
	system("kill -9 $(pgrep chrome)")
	system('kill -9 $(ps aux | grep "measureTemp.rb" | awk \'FNR<2 {print $2}\')')
	system('killall power') if thereIsPhidget

 	interference(domain,resFile,time)  		# (5) pi digits calculation
break
}
