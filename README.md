# TRACEROUTE
#### Trace the route to a host

## What is traceroute?
traceroute prints a trace of the route IP packets are travelling to a remote host. 

## Make Commands

### make or make all
"make" and "make all" compile the project.

### make clean
Removes object files.

### make fclean
Deletes all produced files.

### make re
Deletes all produced files and recompiles again.

## Usage

### Default Usage
./traceroute x.x

is equivalent to

traceroute x.x -p 33434 -f 1 -m 30 -q 3 -t 0 -w 3 -U

## Options

### -p : port : (default for UDP: 33434, TCP: 80, ICMP: no port.)
Sets port to connect to. ICMP doesn't have a port.

### -f : first hop (default: 1)
If you want you can pass the next "-f" hops.

### -n : no resolve host (default: true)
Do not resolve hostnames.

### -m : max hop number (default: 30)
How many hops do you see?

### -q : packet number (default: 3)
How many probes do you want to send?

### -t : tos number (default: 0)
Tos is the "Quality of Service" value. Allows you to set packet priority.
! Provide the TOS value in decimal form.

### -w : wait time (default: 3)
Wait time per probe.

### -T : TCP (default: false)
Use TCP.

### -U : UDP (default: true)
Use UDP.

### -I : ICMP (default: false)
Use ICMP.