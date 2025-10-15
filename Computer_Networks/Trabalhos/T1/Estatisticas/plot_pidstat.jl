using Plots

log_file = "pidstat_log.txt"

cpu_timestamps = String[]
io_timestamps = String[]
cpu_system_usages = Float64[]
cpu_user_usages = Float64[]
cpu_guest_usage = Float64[]
cpu_wait = Float64[]
cpu_total_usage = Float64[]
amount_of_cpus = Int64[] 
read_kbs = Float64[]
write_kbs = Float64[]
kB_ccwr = Float64[]
iodelay = Float64[]

mode = ""  # Can be "cpu" or "io"

f = open(log_file, "r")

for line in eachline(f)
    if occursin("%CPU", line)
        global mode = "cpu"
        continue
    elseif occursin("kB_rd/s", line)
        global mode = "io"
        continue
    end
    if occursin(r"^\d{2}:\d{2}:\d{2}", line)
        parts = split(line)
        timestamp = parts[1]
        if mode == "cpu"
            push!(cpu_timestamps, timestamp)
            push!(cpu_user_usages, parse(Float64, parts[5]))
            push!(cpu_system_usages, parse(Float64, parts[6]))
            push!(cpu_guest_usage, parse(Float64, parts[7]))
            push!(cpu_wait, parse(Float64, parts[8]))
            push!(cpu_total_usage, parse(Float64, parts[9]))
            push!(amount_of_cpus, parse(Int64, parts[10]))
        elseif mode == "io"
            push!(io_timestamps, timestamp)
            push!(read_kbs, parse(Float64, parts[5]))
            push!(write_kbs, parse(Float64, parts[6]))
            push!(kB_ccwr, parse(Float64, parts[7]))
            push!(iodelay, parse(Float64, parts[8]))
        end
    end
end

function save_plot(y, ylabel::String, filename::String; xlabel="Sample", title=ylabel * " Over Time")
    x = 1:length(y)
    p = plot(x, y, xlabel=xlabel, ylabel=ylabel, title=title)
    savefig(filename)
end

# Generate and save plots
save_plot(cpu_total_usage, "% CPU Total", "Total_CPU_usage.png")
save_plot(cpu_user_usages, "% CPU User", "CPU_User_usage.png")
save_plot(cpu_system_usages, "% CPU System", "CPU_System_usage.png")
save_plot(cpu_guest_usage, "% CPU Guest", "CPU_Guest_usage.png")
save_plot(amount_of_cpus, "% CPU Amount", "CPU_Amout_wait.png")
save_plot(cpu_wait, "% CPU I/O Wait", "CPU_IO_wait.png")

save_plot(read_kbs, "kB Read/s", "IO_Read_kbs.png")
save_plot(write_kbs, "kB Write/s", "IO_Write_kbs.png")
save_plot(kB_ccwr, "kB Cancelled Write/s", "IO_Cancelled_Write_kbs.png")
save_plot(iodelay, "IO Delay", "IO_Delay.png")
