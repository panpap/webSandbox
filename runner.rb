require 'optparse'

def cpuMemTest(resFile)
	#pid=`ps aux | grep "chrome --type=renderer" | awk 'FNR>1 {print $2}'`.split("\n").first
	`ps aux | grep "chrome --type=renderer"`.split("\n").each{|line|
		if line.include? ";" and line.size>150
			pid=line.split(" ")[1]
			puts "\n> Measuring CPU and MEM in pid: #{pid}..."
			system("psrecord --include-children --interval 1 --plot #{resFile}#{pid}_memCPU.png --log #{resFile}#{pid}_memCPU.log #{pid} &")
		end
	}
end

def interference(domain,resFile,t)
	puts "> Probe (No2) with inteference..."
	system("google-chrome-stable --incognito --no-sandbox --disable-extensions http://#{domain} > /dev/null 2>&1 &")
	system("./tests/interference/y-cruncher-v0.7.5.9480-static/y-cruncher custom pi -dec:1b > #{resFile}_interference.log &")
	sleep(120)
	system("kill -9 $(ps aux | grep y-cruncher | grep \"custom pi -dec:1\" | awk '{print $2}')")
	system("kill -9 $(pgrep chrome)")
end

def temperature(resFile)
	system("ruby ./tests/temperature/measureTemp.rb 1 #{resFile} &")
end

def power(resFile)
	puts "> Measuring power..."
	system("./tests/power/power > #{resFile}_power.csv &")
end

def getHar(resFile,domain,time)
	timeout=1000*time
	system("chrome-har-capturer --grace #{timeout} --host 127.0.0.1 -c -o #{resFile}_requests.har http://#{domain} &")
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
thereIsPhidget=false
if `if [ -f tests/power/power ]; then echo "exists" ; fi;`.include? "exists"
	thereIsPhidget=true
else
	puts "Warning: No sensor found. Proceeding without measuring power..."
end
count=0
start = Time.now
#start mining probes
doms.each{|dom|
	domain=dom.gsub("\n","")
	puts "Probing "+domain
	filename=domain.gsub("\n","").gsub("/","-")
	headDir=dir+filename
	resFile=headDir+"/"+filename
	system("mkdir -p #{headDir}/memCPU/")
	puts "> Opening Chrome..."
	system("google-chrome-stable --incognito --headless --disable-extensions --no-sandbox --remote-debugging-port=9222 > /dev/null 2>&1 &")
	sleep(2)
	#run Tests
	power(resFile) if thereIsPhidget	    # (1) system power 
	temperature(headDir)   # (2) system temperature
	getHar(resFile,domain,time) 					# (3) get har
	sleep(0.5)
	cpuMemTest("#{headDir}/memCPU/"+filename) 					# (4) cpu & mem
	sleep(time)
	print "waiting"
	tasks=`ps aux | grep chrome-har-capturer | wc -l`.to_i
	prevTask=tasks
	if tasks>2
		c=0
		while(prevTask== tasks and c<100) # wait till close
			puts c
			sleep(2)
			c+=1
			tasks=`ps aux | grep chrome-har-capturer | wc -l`.to_i
		end
	end
	puts("\nclosing")
	system("kill -9 $(pgrep chrome)")
	#system('kill -9 $(ps aux | grep measureTemp | awk \'FNR<2 {print $2}\')')
	`ps aux | grep measureTemp`.split("\n").each{|line|
		pid=line.split(" ")[1]
		system('kill -9 '+pid)	
	}
	system('killall power') if thereIsPhidget

 	interference(domain,resFile,time)  		# (5) pi digits calculation
	count+=1
	system("rm -f *.txt")
	system("tar -zcf  #{headDir}.tar.gz #{headDir}; rm -rf #{headDir}/")
	puts "Completed #{count}/#{doms.size} in "+(Time.now-start).to_s+"sec" if count%200==0
}
puts "Total elapsed time: "+(Time.now-start).to_s+"sec"
