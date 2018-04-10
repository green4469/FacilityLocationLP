import glob

def parse_input_file(fname):
	f = open(fname, 'r')
	lines = f.readlines()
	for i, line in enumerate(lines):
		if i == 2:
			dim_and_opening_cost = [int(s) for s in line.split() if s.isdigit()]
			dim = dim_and_opening_cost[0]
			n_facility = dim
			n_client = dim
			opening_cost = [dim_and_opening_cost[1] for i in range(n_facility)]
			connection_cost = [[-1 for j in range(n_client)] for i in range(n_facility)]
			continue
		elif i < 4:
			continue
		ftc = [int(s) for s in line.split() if s.isdigit()]
		connection_cost[ftc[0]-1][ftc[1]-1] = ftc[2]
	f.close()
	removed_clients = []
	for j in range(n_client):
		not_connected = 0
		for i in range(n_facility):
			if connection_cost[i][j] == -1:
				not_connected += 1
		print(not_connected)
		if not_connected == n_facility:
			removed_clients.append(j)
	removed_clients.sort()
	for i, c in enumerate(removed_clients):
		n_client -= 1
		for row in connection_cost:
			del row[c - i]
	return n_facility, n_client, opening_cost, connection_cost, removed_clients

def write_file(fname, n_facility, n_client, opening_cost, connection_cost, removed_clients):
	f = open(fname, 'w+')
	f.write('%d\n'%(n_facility))
	f.write('%d\n'%(n_client))
	opening_cost_str = ''
	for facility in range(n_facility):
		opening_cost_str += '%d '%(opening_cost[facility])
	opening_cost_str = opening_cost_str[:-1] + '\n'
	f.write(opening_cost_str)
	for facility in range(n_facility):
		for client in range(n_client):
			if connection_cost[facility][client] == -1:
				continue
			f.write('%d %d %d\n'%(facility, client, connection_cost[facility][client]))
	f.close()
	return None

for curr_dir in glob.glob('C:\\Users\\pyeon\\workspace\\FacilityLocationLP\\nsc\\*'):
	curr_class = curr_dir.split('\\')[-1]
	if curr_class == 'gapA':
		start_idx = 0
	elif curr_class == 'gapB':
		start_idx = 50
	elif curr_class == 'gapC':
		start_idx = 100
	else:
		continue
	input_files = glob.glob(curr_dir+'\\*Gap*.txt')
	for i, input_file in enumerate(input_files):
		n_facility, n_client, opening_cost, connection_cost, removed_clients = parse_input_file(input_file)
		#print(len(removed_clients))
		prob_idx = int(input_file.split('\\')[-1].split('G')[-2]) // 100 + start_idx
		fname = curr_dir + '\\FLP_IN_'+'%04d'%(prob_idx)+'.txt'
		write_file(fname, n_facility, n_client, opening_cost, connection_cost, removed_clients)
