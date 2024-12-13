import requests
import sys
import os

URL: str = "https://adventofcode.com"


if __name__ == "__main__":

    session_token = os.getenv("AOC_TOKEN")

    # Must have token

    if session_token is None:
        print("No token.")
        sys.exit(1)

    year = int(sys.argv[1])
    day = int(sys.argv[2])

    url = f"{URL}/{year}/day/{day}/input"
    headers = {"Cookie": f"session={session_token}"}
    r = requests.get(url, headers=headers)

    # Must be successful

    if r.status_code == 200:
        print(r.text, end="")
    else:
        print(f"Uh oh: {r.status_code}")
        sys.exit(1)
