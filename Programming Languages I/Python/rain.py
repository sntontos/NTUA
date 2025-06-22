import sys

def read_file(filename): # Reads a file and returns a list of integers
    with open(filename, 'r') as file:
        data = file.readlines()
    if not data or not data[0].strip():
        raise ValueError("File is empty or contains no valid data.")
    try:
        values = list(map(int, data[0].split()))
    except ValueError:
        raise ValueError("File contains non-integer values.")
    return values

def get_heights(): # Gets the heights from a file specified in the command line argument
    if len(sys.argv) != 2:
        print("Usage: python rain.py <filename>")
        sys.exit(0)

    filename = sys.argv[1]
    try:
        rainfall_data = read_file(filename)
        if not rainfall_data:
            print("No data found in the file.")
            return []
        return rainfall_data
    except FileNotFoundError:
        print(f"File '{filename}' not found.")
    except ValueError as e:
        print(f"Error processing file: {e}")

def calculate_water_volume(heights): # Calculates the total volume of water that can be trapped
    if not heights:
        return 0
    if any(h < 0 for h in heights):
        raise ValueError("Heights must be non-negative integers.")
    if len(heights) < 3:
        print("Need at least 3 values to trap water.")
        return 0

    left, right = 0, len(heights) - 1
    left_max, right_max = heights[left], heights[right]
    water_volume = 0
    #Changed to pointer based approach for space complexity optimization
    while left < right:
        if heights[left] < heights[right]:
            left += 1
            left_max = max(left_max, heights[left])
            water_volume += max(0, left_max - heights[left])
        else:
            right -= 1
            right_max = max(right_max, heights[right])
            water_volume += max(0, right_max - heights[right])

    return water_volume

if __name__ == "__main__":
    heights = get_heights()
    print(type(heights))
    if heights:
        total_volume = calculate_water_volume(heights)
        print(total_volume)
    else:
        print("No heights provided to calculate water volume.")
    sys.exit(0)
  
