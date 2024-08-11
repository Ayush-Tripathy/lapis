from time import time

FILENAME = "datasets/Spotify_Youtube.csv"


import lapis as lp

t1 = time()

df = lp.read_csv(FILENAME)
print(df)

t2 = time()
print(f"Time taken [Lapis]: {t2 - t1}")


# import pandas as pd

# t1 = time()

# df = pd.read_csv(FILENAME)
# print(df)

# t2 = time()
# print(f"Time taken [Pandas]: {t2 - t1}")

# import polars as pl

# t1 = time()

# df = pl.read_csv(FILENAME)
# print(df)

# t2 = time()
# print(f"Time taken [Polars]: {t2 - t1}")
