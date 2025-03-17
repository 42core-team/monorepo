import random

def generate_line() -> str:
	return ''.join(random.choice('123456789') for _ in range(4))

def create_files():
	for num in range(26):
		filename = f"wall_tiny_{num}.txt"
		try:
			with open(filename, 'w') as file:
				for _ in range(4):
					file.write(generate_line() + "\n")
			print(f"Created file: {filename}")
		except IOError as e:
			print(f"Error creating file {filename}: {e}")

if __name__ == "__main__":
	create_files()
