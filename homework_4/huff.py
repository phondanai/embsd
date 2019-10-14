import sys
import struct
import pickle
import pprint


DEBUG = True


def get_code_length(byte_string):
    return struct.unpack("I", byte_string[-4:])[0]


def int_to_bin_string(b):
    return "{:08b}".format(b)


def encode(string, codes):
    output = ""
    for ch in string:
        output += codes[ch]
    return output


def bitstring_to_bytes(s):
    return int(s, 2)


class NodeTree(object):
    def __init__(self, left=None, right=None):
        self.left = left
        self.right = right

    def children(self):
        return (self.left, self.right)

    def nodes(self):
        return (self.left, self.right)

    def __str__(self):
        return "{} {}".format(self.left, self.right)


# Traverse the NodeTree in every possible way to get codings
def huffmanCodeTree(node, left=True, binString=""):
    if type(node) is int:
        return {node: binString}
    (l, r) = node.children()

    d = dict()
    d.update(huffmanCodeTree(l, True, binString + "0"))
    d.update(huffmanCodeTree(r, False, binString + "1"))

    return d


if __name__ == "__main__":

    if len(sys.argv) < 2:
        print("Please input file name")
        sys.exit(1)

    input_file = sys.argv[1]

    with open(input_file, "rb") as f:
        string = f.read()

    freq = {}
    for c in string:
        if c in freq:
            freq[c] += 1
        else:
            freq[c] = 1

    # sort the freq table based on occurrence
    freq = sorted(freq.items(), key=lambda x: x[1], reverse=True)

    if DEBUG:
        print(" Char | Freq ")
        for key, c in freq:
            print(" %4r | %d" % (key, c))
        print("-" * 30)

    nodes = freq

    while len(nodes) > 1:
        key1, c1 = nodes[-1]
        key2, c2 = nodes[-2]
        nodes = nodes[:-2]

        node = NodeTree(key1, key2)
        nodes.append((node, c1 + c2))

        nodes = sorted(nodes, key=lambda x: x[1], reverse=True)

    if DEBUG:
        print("left: %s" % nodes[0][0].nodes()[0])
        print("right: %s" % nodes[0][0].nodes()[1])

    huffmanCode = huffmanCodeTree(nodes[0][0])
    if DEBUG:
        print(" Char | Freq | Huffman code ")
        print("-" * 30)
        for char, frequency in freq:
            print(" %-4r | %5d | %12s" % (char, frequency, huffmanCode[char]))

    encoded_string = encode(string, huffmanCode)

    encoded_list = [encoded_string[i : i + 8] for i in range(0, len(encoded_string), 8)]
    encoded_bits = len(encoded_string)

    if len(encoded_list[-1]) != 8:
        encoded_list[-1] = encoded_list[-1].ljust(8, "0")

    encoded_list.append("{0:32b}".format(encoded_bits))

    # write Huffman table
    with open("{}".format(input_file) + ".huff", "wb") as huffman_table:
        pickle.dump(huffmanCode, huffman_table)

    # Write compressed file
    with open("{}".format(input_file) + ".compress", "wb") as compressed:
        print("Writing compress file: {}".format(input_file + ".compress"))
        for i in encoded_list[:-1]:
            chunk = struct.pack("B", bitstring_to_bytes(i))
            compressed.write(chunk)
        no_bits = struct.pack("I", bitstring_to_bytes(encoded_list[-1]))
        compressed.write(no_bits)

    # read Huffman table
    with open("{}".format(input_file) + ".huff", "rb") as huffman_table_file:
        huffman_table = pickle.load(huffman_table_file)

    decode_dict = {k: v for v, k in huffman_table.items()}

    # Decompressed file
    with open("{}".format(input_file) + ".compress", "rb") as decompressed:
        byte_s = decompressed.read()

    code_length = get_code_length(byte_s)
    code_string = "".join(["{0:08b}".format(i) for i in byte_s])

    key = ""
    #ff = ""
    ff = []
    for i in code_string[:code_length]:
        key += i
        if key in decode_dict:
            #ff += decode_dict[key]
            ff += [decode_dict[key]]
            key = ""

    with open("{}".format(input_file) + ".decompress", "wb") as decompressed_file:
        decompressed_file.write(bytes(ff))
