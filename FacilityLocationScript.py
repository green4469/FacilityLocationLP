#MultiwayCutScript.py
import sys
import subprocess
import glob
import pandas

prob = 'FLP'

def execute(fname, src):
	input_files = glob.glob(src+prob+'_IN*')
	input_files.sort()
	for f in input_files:
		command = '{} {}'.format(fname, f)
		subprocess.call(command)


def process_output(src):
	output_files = glob.glob(src+prob+'_OUT*')
	output_files.sort()
	df = pd.DataFrme(columns=['problem, relaxed, rounded, sol'])
	for f in output_files:
		prob_idx = int(f.split('\\')[-1].split('.')[-2].split('_')[-1])
		cur_df = pd.read_csv(f)
		cur_df['problem'] = [prob_idx]
		df.append(cur_df)
	pd.to_csv(src+'results.csv', index=False)


def main():
	fname = 'MultiwayCut.exe'
	if len(sys.argv) > 1:
	src = sys.argv[1]
	if src[-1] != '\\'
		src += '\\'
	else:
		src = '.\\'
	execute(fname, src)
	process_output(src)

if __name__ == '__main__':
	main()