import matplotlib.pyplot as plt
import argparse
import re

def parser_input(filename):
    with open(filename, 'r') as file:
        lines = file.readlines()
    
    die_size = [int(x) for x in lines[4].split()[1:]]
    inputPins = []
    outputPins = []
    cellLibrary = {}
    input_instances = {}
    nets = []
    util_ratio_info = []
    placement_rows = []
    gates = {}
    
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
            cell_pins = []
            for _ in range(num_pins):
                idx += 1
                pin_info = lines[idx].split()
                cell_pins.append((pin_info[1], float(pin_info[2]), float(pin_info[3])))
            cellLibrary[cell_name] = (cell_width, cell_height, num_pins, cell_pins,-1)
            idx += 1
            
    instance_start_idx = idx
    num_instances = int(lines[instance_start_idx].split()[1])
    for i in range(num_instances):
        parts = lines[instance_start_idx + 1 + i].split()
        instance_name = parts[1]
        instance_type = parts[2]
        
        instance_x = float(parts[3])
        instance_y = float(parts[4])
        if(cellLibrary[instance_type][4] == -1):#gate
            input_instances[instance_name] = (instance_type, instance_x, instance_y,True)   
        else:#flipflop
            input_instances[instance_name] = (instance_type, instance_x, instance_y,False)  
        
        idx += 1
    
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
        
    return die_size, inputPins, outputPins, cellLibrary, input_instances, nets, util_ratio_info, placement_rows, gates
    
    
    
def parser_output(filename, input_instances):
    with open(filename, 'r') as file:
        lines = file.readlines()
    output_instances = {}
    
    num_instances = int(lines[0].split()[1])
    for i in range(num_instances):
        parts = lines[1 + i].split()
        instance_name = parts[1]
        instance_type = parts[2]
        instance_x = float(parts[3])
        instance_y = float(parts[4])
        output_instances[instance_name] = (instance_type, instance_x, instance_y)
    idx = num_instances + 1
    return output_instances

def plot_output(die_size, cellLibrary, input_instances, output_instances, nets, placement_rows,inputPins,outputPins):
    print("Plotting output design...")
    plt.figure(figsize=(12, 12))
    plt.title("Input Design Placement")
    plt.xlim(die_size[0], die_size[2])
    plt.ylim(die_size[1], die_size[3])
    
    for name, x, y in inputPins:
        plt.scatter(x, y, color='#008000',s = 3.7, label='Input Pins')
    print("check7")

    for name, x, y in outputPins:
        plt.scatter(x, y, color='navy',s = 3.7, label='Output Pins')
    print("check8")
    
    num_gate_plotted = 0
    for instance in input_instances.items():#plot gate only
        if(instance[1][3] == True):
            inst_type = instance[1][0]
            inst_x = instance[1][1]
            inst_y = instance[1][2]
            cell_width = cellLibrary[inst_type][0]
            cell_height = cellLibrary[inst_type][1]
            rect = plt.Rectangle((inst_x, inst_y), cell_width, cell_height, fill=False, edgecolor='black', label='Gates' if num_gate_plotted == 0 else "")
            plt.gca().add_patch(rect)
            num_gate_plotted += 1
        else:
            continue

    print('number of gate   ',num_gate_plotted)
    # plt.legend()
    plt.xlabel('X-coordinate')
    plt.ylabel('Y-coordinate')
    plt.axis('equal')
    
    # Save the plot to a file instead of showing it
    output_filename = 'placement_result.png'
    plt.savefig(output_filename, dpi=300, bbox_inches='tight')
    print(f"Plot has been saved to {output_filename}")
    plt.close()

def main():
    parser = argparse.ArgumentParser(description='Plot the result of the placement')
    parser.add_argument('input_file', type=str, help='Path to the input file')
    parser.add_argument('output_file', type=str, help='Path to the output file')
    args = parser.parse_args()
    
    
    die_size, inputPins, outputPins, cellLibrary, input_instances, nets, util_ratio_info, placement_rows, gates = parser_input(args.input_file)
    
    output_instances = parser_output(args.output_file, input_instances)
    
    plot_output(die_size,cellLibrary, input_instances, output_instances, nets, placement_rows,inputPins,outputPins)
    
    
if __name__ == '__main__':
    main()
