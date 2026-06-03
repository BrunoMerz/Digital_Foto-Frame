import gzip  
with open("./index.html", "rb") as f:
    html = f.read()

gz = gzip.compress(html, compresslevel=9)

with open("index_html.h", "w") as f:
    f.write(f"const uint8_t MZ_HTML[{len(gz)}] PROGMEM = {{\n")

    for i, b in enumerate(gz):
        if i % 16 == 0:
            f.write("    ")

        f.write(f"{b},")

        if i % 16 == 15:
            f.write("\n")

    f.write("\n};\n")
	