import matplotlib.pyplot as plt
import argparse

# Function to parse input file
def parse_input(filename):
    with open(filename, 'r') as file:
        lines = file.readlines()

    die_size = list(map(int, lines[4].split()[1:]))
    inputPins = []
    # inputPins[id] = (pin_name, x, y)
    outputPins = []
    # outputPins[id] = (pin_name, x, y)
    cellLibrary = {}
    # cellLibrary[flipflop_name] = (cell_width, cell_height, num_pins, cell_pins, num_bits, Qpin_delay, gate_power)
    # cellLibrary[gate_name] = (cell_width, cell_height, num_pins, cell_pins, gate_power)
    input_instances = {}
    # input_instances[instance_name] = (instance_type, x, y, timing_slack)
    nets = []
    # nets[id] = (net_name, num_pins, net_pins)
    # net_pins[id] = (instance_name, pin_name) or (input_pin) or (output_pin)
    util_ratio_info = []
    # util_ratio_info = [bin_width, bin_height, bin_max_util_ratio]
    placement_rows = []
    # placement_rows[id] = (start_x, start_y, site_width, site_height, num_sites)

    num_inputPins = int(lines[5].split()[1])
    inputPin_start_idx = 6
    for i in range(num_inputPins):
        parts = lines[inputPin_start_idx + i].split()
        inputPins.append((parts[1], float(parts[2]), float(parts[3])))

    num_outputPins = int(lines[inputPin_start_idx + num_inputPins].split()[1])
    outputPin_start_idx = inputPin_start_idx + 1 + num_inputPins
    for i in range(num_outputPins):
        parts = lines[outputPin_start_idx + i].split()
        outputPins.append((parts[1], float(parts[2]), float(parts[3])))

    cellLibrary_start_idx = outputPin_start_idx + num_outputPins
    idx = cellLibrary_start_idx
    while lines[idx].startswith("FlipFlop") or lines[idx].startswith("Gate"):
      if lines[idx].startswith("FlipFlop"):
        cell_info = lines[idx].split()
        cell_pins = []
        num_bits = int(cell_info[1])
        cell_name = cell_info[2]
        cell_width = float(cell_info[3])
        cell_height = float(cell_info[4])
        num_pins = int(cell_info[5])
        for _ in range(num_pins):
            idx += 1
            pin_info = lines[idx].split()
            cell_pins.append((pin_info[1], float(pin_info[2]), float(pin_info[3])))
        cellLibrary[cell_name] = (cell_width, cell_height, num_pins, cell_pins, num_bits)
        idx += 1
      elif lines[idx].startswith("Gate"):
        cell_info = lines[idx].split()
        cell_name = cell_info[1]
        cell_width = float(cell_info[2])
        cell_height = float(cell_info[3])
        num_pins = int(cell_info[4])
        for _ in range(num_pins):
            idx += 1
            pin_info = lines[idx].split()
            cell_pins.append((pin_info[1], float(pin_info[2]), float(pin_info[3])))
        cellLibrary[cell_name] = (cell_width, cell_height, num_pins, cell_pins, 0)
        idx += 1
    
    instance_start_idx = idx
    num_instances = int(lines[instance_start_idx].split()[1])
    for i in range(num_instances):
        parts = lines[instance_start_idx + 1 + i].split()
        instance_name = parts[1]
        instance_type = parts[2]
        instance_x = float(parts[3])
        instance_y = float(parts[4])
        input_instances[instance_name] = (instance_type, instance_x, instance_y, 0.0)
        idx += 1
    print("check1") 
    
    idx += 1
    nets_start_idx = idx
    num_nets = int(lines[nets_start_idx].split()[1])
    for i in range(num_nets):
        idx += 1
        parts = lines[idx].split()
        net_name = parts[1]
        num_pins = int(parts[2])
        net_pins = []
        for _ in range(num_pins):
            idx += 1
            pin_info = lines[idx].split()
            pin_info = pin_info[1].split('/')
            net_pin = []
            net_pin.append((pin_info[0]))
            if len(pin_info) > 1:
                net_pin.append((pin_info[1]))
            net_pins.append(net_pin)
        nets.append((net_name, num_pins, net_pins))
        
    idx += 1
    print("check2")
    util_ratio_start_idx = idx
    for i in range(3):
        parts = lines[util_ratio_start_idx + i].split()
        util_ratio_info.append(float(parts[1]))
        idx += 1
        
    idx += 1
    while lines[idx].startswith("PlacementRows"):
        parts = lines[idx].split()
        start_x = float(parts[1])
        start_y = float(parts[2])
        site_width = float(parts[3])
        site_height = float(parts[4])
        num_sites = int(parts[5])
        placement_rows.append((start_x, start_y, site_width, site_height, num_sites))
        idx += 1
    print("check3")
    
    idx += 1    
    while lines[idx].startswith("QpinDelay"):
        parts = lines[idx].split()
        cell_name = parts[1]
        Qpin_delay = float(parts[2])
        cellLibrary[cell_name] = cellLibrary[cell_name] + (Qpin_delay,)
        idx += 1
    print("check4")
    
    while lines[idx].startswith("TimingSlack"):
        parts = lines[idx].split()
        instance_name = parts[1]
        slack_pin = parts[2]
        timing_slack = float(parts[3])
        input_instances[instance_name] = input_instances[instance_name] + (timing_slack,)
        idx += 1
    print("check5")
        
    while lines[idx].startswith("GatePower"):
        parts = lines[idx].split()
        cell_name = parts[1]
        gate_power = float(parts[2])
        cellLibrary[cell_name] = cellLibrary[cell_name] + (gate_power,)
        idx += 1
        if idx >= len(lines):
            break
    print("check6")
    
    # print("Die Size:", die_size)
    # print("Input Pins:", inputPins)
    # print("Output Pins:", outputPins)
    # print("Cell Library:", cellLibrary)
    # print("Input Instances:", input_instances)
    # print("Nets:", nets)
    # print("Util Ratio Info:", util_ratio_info)
    # print("Placement Rows:", placement_rows)
        
    return die_size, inputPins, outputPins, cellLibrary, input_instances, nets, util_ratio_info, placement_rows

# Function to plot the design
def plot_input_design(die_size, inputPins, outputPins, cellLibrary, input_instances, nets, util_ratio_info, placement_rows):
    print("Plotting input design...")
    plt.figure(figsize=(12, 12))
    plt.title("Input Design Placement")
    plt.xlim(die_size[0], die_size[2])
    plt.ylim(die_size[1], die_size[3])

    # Plot input pins
    for name, x, y in inputPins:
        plt.scatter(x, y, color='blue',s = 10)
       # plt.text(x, y)
    print("check7")

    # Plot output pins
    for name, x, y in outputPins:
        plt.scatter(x, y, color='green',s = 10)
        #plt.text(x, y)
    print("check8")

    # Plot instances
    for inst_name in input_instances.items():
        inst_type = inst_name[1][0]
        inst_x = inst_name[1][1]
        inst_y = inst_name[1][2]
        
        cell_width = cellLibrary[inst_type][0]
        cell_height = cellLibrary[inst_type][1]
        #for pin_name, pin_dx, pin_dy in cellLibrary[inst_type][3]:
           # pin_x = inst_x + pin_dx
            #pin_y = inst_y + pin_dy
            #plt.scatter(pin_x, pin_y, color='red')
            #plt.text(pin_x, pin_y, f'{inst_name}/{pin_name}', fontsize=8)
        if(cellLibrary[inst_type][4] > 0):
            rect = plt.Rectangle((inst_x, inst_y), cell_width, cell_height, fill=True, edgecolor='black')
        elif(cellLibrary[inst_type][4] == 0):
            rect = plt.Rectangle((inst_x, inst_y), cell_width, cell_height, fill=True, edgecolor='purple')
        plt.gca().add_patch(rect)
        #print("check9")
    print("check10")

    plt.legend()
    plt.xlabel('X-coordinate')
    plt.ylabel('Y-coordinate')
    plt.axis('equal')
    #plt.grid(True)
    plt.show()

def main():
    parser = argparse.ArgumentParser(description='Plot input and output design placements.')
    parser.add_argument('input_file', type=str, help='Path to the input file')
    parser.add_argument('output_file', type=str, help='Path to the output file')

    args = parser.parse_args()

    die_size, inputPins, outputPins, cellLibrary, input_instances, nets, util_ratio_info, placement_rows = parse_input(args.input_file)

    # Plot input design
    plot_input_design(die_size, inputPins, outputPins, cellLibrary, input_instances, nets, util_ratio_info, placement_rows)

if __name__ == '__main__':
    main()