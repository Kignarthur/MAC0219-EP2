import re
import csv
import pandas as pd
from matplotlib import pyplot as plt

class Mandelbroter():
    """ Creates csv file and plots the data """
    def __init__(self, header, exe_type, file_path, file_name):
        self.header = header
        self.exe_type = exe_type
        self.file_path = file_path
        self.file_name = file_name

    def process_data(self, is_sequential):
        self.create_csv_file()

        if is_sequential:
            self.plot_sequential()
        else:
            self.plot_parallel()

    def create_csv_file(self):
        log_file = self.file_path + '.log'
        csv_file = self.file_path + '.csv'

        # Get all lines
        with open(log_file, "r") as file:
            lines = file.readlines()

        # Write to csv
        with open(csv_file, "w") as csvf:
            writer = csv.writer(csvf)
            writer.writerow(self.header)

            row = []
            N_PROCS = 3
            N_THREADS = 10

            run_offset = 1
            time_offset = 12
            space_between_measurements = 14

            for line_number, line in enumerate(lines):
                # Get number of processes and number of threads
                if (line_number - run_offset) % space_between_measurements == 0:
                    parameters = re.findall("'(.*?)'", line, re.DOTALL)[0].split()
                    row += [parameters[N_PROCS], parameters[N_THREADS]]

                # Get execution time and confidence interval
                elif (line_number - time_offset) % space_between_measurements == 0:
                    times = re.findall("\d+\.\d+", line)
                    row += times[:2]

                    writer.writerow(row); row = []

    def plot_sequential(self):
        df = pd.read_csv(self.file_path + '.csv')

        x_coord  = self.header[0]
        y_coord  = self.header[2]
        interval = self.header[3]

        x = df[x_coord].astype("string")
        y = df[y_coord]
        i = df[interval]

        plt.bar(x, y, yerr=i, capsize=7, width=0.25, color='royalblue', zorder=3)

        # Wrap up
        self.put_essential_labels(xlabel=x_coord, ylabel=y_coord, axis='y')
        self.save_plt()

    def plot_parallel(self):
        df = pd.read_csv(self.file_path + '.csv')
        group_header = self.header[0]
        x_coord  = self.header[1]
        y_coord  = self.header[2]
        interval = self.header[3]

        groups = df.groupby(group_header)

        for group, _ in groups:
            data = groups.get_group(group).convert_dtypes()
            x = data[x_coord].astype("string")
            y = data[y_coord]
            i = data[interval]
            plt.plot(x, y, label=group, marker='o')
            plt.fill_between(x, (y-i).tolist(), (y+i).tolist(), color='dodgerblue', alpha=0.5)

        # Wrap up
        plt.legend(loc='upper right', bbox_to_anchor=(1.2, 1), title=group_header)
        self.put_essential_labels(xlabel=x_coord, ylabel=y_coord)
        self.save_plt()

    def put_essential_labels(self, xlabel="", ylabel="", axis='both'):
        plt.xlabel(xlabel)
        plt.ylabel(ylabel)
        plt.title(f'{self.file_name} using {self.exe_type}')
        plt.grid(ds='steps-post',axis=axis)

    def save_plt(self):
        plot_path = self.file_path + '.png'
        plt.savefig(plot_path, bbox_inches='tight')
        plt.clf()

def main():
    # All possible files
    results_dir = '../data/'
    dir_names = [f'mandel_ompi_{exec}/' for exec in ['seq', 'pth', 'omp']]
    file_names = ['triple_spiral']

    # Naming
    header = ['Processes', 'Threads', 'Execution Time (s)', 'Confidence Interval']
    exe_types = ['OMPI', 'OMPI + Pthreads', 'OMPI + OMP']
    formated_fnames = ['Triple Spiral Valley']

    # Permutate and process
    for d, dir_name in enumerate(dir_names):
        for f, file_name in enumerate(file_names):
            exe_type = exe_types[d]
            formated_fname = formated_fnames[f]
            file_path = results_dir + dir_name + file_name

            mandel = Mandelbroter(header, exe_type, file_path, formated_fname)
            mandel.process_data(is_sequential=(dir_name == dir_names[0]))

if __name__ == '__main__':
    main()
