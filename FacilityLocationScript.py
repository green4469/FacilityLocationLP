#MultiwayCutScript.py
import sys
import subprocess
import glob
import pandas as pd

prob = 'FLP'


def execute(fname, src):
	input_files = glob.glob(src+'*.txt')
	input_files.sort()
	for f in input_files:
		command = '{} {}'.format(fname, f)
		print(command)
		subprocess.call(command)


def process_output(src):
	output_files = glob.glob(src+prob+'_OUT\\*.txt')
	output_files.sort()
	df = pd.DataFrame(columns=['problem', 'relaxed', 'rounded'])
	print(output_files)
	for fname in output_files:
		prob_idx = int(fname.split('\\')[-1].split('.')[-2].split('_')[-1])
		f = open(fname, 'r')
		relaxed = int(f.readline())
		rounded = int(f.readline())
		f.close()
		#print(relaxed)
		#cur_df = pd.read_csv(f)
		#print(cur_df)
		#cur_df['problem'] = [prob_idx]
		df = df.append({'problem':prob_idx, 'relaxed':relaxed, 'rounded':rounded}, ignore_index=True)
		print(df)
	df.to_csv(src+'results.csv', index=False)


def main():
	fname = 'FacilityLocation.exe'
	if len(sys.argv) > 1:
		src = sys.argv[1]
		if src[-1] != '\\':
			src += '\\'
	else:
		src = '.\\'
	execute(fname, src)
	#process_output(src)

if __name__ == '__main__':
	main()
