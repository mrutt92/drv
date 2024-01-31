import pandas as pd
import argparse

pd.set_option('display.max_rows', None)

parser = argparse.ArgumentParser(description='Show tag execution')
parser.add_argument('stats_csv', default='stats.csv', type=str, help='stats CSV')
parser.add_argument('tags_csv', default='tags.csv', type=str, help='tags CSV')
parser.add_argument('--start-tag', type=str, default="", help='start tag')
parser.add_argument('--end-tag', type=str, default="", help='end tag')

args = parser.parse_args()

data = pd.read_csv(args.stats_csv)
tags = pd.read_csv(args.tags_csv)

start_tag = tags[tags['TagName']==args.start_tag]
stop_tag  = tags[tags['TagName']==args.end_tag]

data = data[data['StatisticName']=='tag_cycles']

keys = ['ComponentName','StatisticSubId']
columns = [] + keys
bins = ["Bin{}:{}-{}.u64".format(i, i, i) for i in range(4)]
columns += bins

# data['SumTotal'] = data[bins].sum(axis=1)
# columns += ['SumTotal']

# binpcts = ["Bin{}:Pct".format(i) for i in range(4)]
# for i in range(4):
#     cname = binpcts[i]
#     data[cname] = data['Bin{}:{}-{}.u64'.format(i, i, i)] / data['SumTotal']

# columns += binpcts

start_data = pd.merge(data, start_tag, on='SimTime')
stop_data  = pd.merge(data, stop_tag, on='SimTime')

full_data = pd.merge(start_data, stop_data, on=['ComponentName','StatisticSubId'], suffixes=('_start', '_stop'))

diffs = []
for i in range(4):
    diff_cname = "Bin{}:{}.diff".format(i, i)
    full_data[diff_cname] = full_data['Bin{}:{}-{}.u64_stop'.format(i, i, i)] - full_data['Bin{}:{}-{}.u64_start'.format(i, i, i)]
    diffs += [diff_cname]

totals = []
full_data["SumTotal"] = full_data[diffs].sum(axis=1)
totals += ["SumTotal"]

pcts = []
for i in range(4):
    pct_cname = "Bin{}:{}.pct".format(i, i)
    full_data[pct_cname] = full_data['Bin{}:{}.diff'.format(i, i)] / full_data['SumTotal']
    pcts += [pct_cname]

columns  = keys
columns += diffs
columns += totals
columns += pcts

print(full_data[columns])

#print(data[columns])
