#!/usr/bin/env python

from __future__ import print_function
import os.path
import sys
try:
    import yaml
except ImportError:
    print('Please install PyYaml')
    sys.exit(1)

# TODO(rqou): Consider the possibility of dynamically generating the
# appropriate Python code to use struct to handle the described data structures
# (rather than just outputting C headers).


def typpo_filename_to_include_guard(fname):
    # Get the "base" (no path, no extension) part of the input name
    fname_base = os.path.splitext(os.path.basename(fname))[0]
    # Take the filename, make it all uppercase, and replace all
    # non-alphanumeric characters with underscores
    fname_cleaned = ''.join(
        c.upper() if c.isalnum() else '_' for c in fname_base)
    return fname_cleaned + "_TYPPO_H_"


def typpo_emit_header_start(f, input_filename):
    HEADER_START_STR = (
        "// DO NOT EDIT THIS FILE!\n"
        "// This file is autogenerated by the typpo type describer tool.\n"
        "// You should edit the source YAML file instead.\n"
        "\n"
        "#ifndef {include_guard}\n"
        "#define {include_guard}\n"
        "\n"
        "#include <stdint.h>\n"
        "\n"
        # This is kinda a hack, but the YAML code will use an alien type called
        # pointer. We add the special-casing for that here instead of in the
        # main type generation.
        "typedef void *pointer;\n"
        "\n")

    include_guard = typpo_filename_to_include_guard(input_filename)
    f.write(HEADER_START_STR.format(include_guard=include_guard))


def typpo_emit_header_end(f, input_filename):
    HEADER_END_STR = (
        "\n"
        "#endif  // {include_guard}\n"
        "\n")

    include_guard = typpo_filename_to_include_guard(input_filename)
    f.write(HEADER_END_STR.format(include_guard=include_guard))


class TyppoFieldRef(object):
    def __init__(self, _name, _typestr):
        self.name = _name
        self.typestr = _typestr

        # Will be filled in with a type if this reference directly embeds
        # a type known to Typpo. Used for DAG linearization
        self.typpo_type = None

    def get_base_type(self):
        base_type = self.typestr
        while True:
            if base_type.endswith("*"):
                # Pointer
                base_type = base_type[:-1].strip()
            elif base_type.endswith("]"):
                # Array
                open_brace_idx = base_type.rfind("[")
                if open_brace_idx == -1:
                    print("ERROR: Mismatched ] in field of type {}"
                          .format(base_type))
                    assert False
                base_type = base_type[:open_brace_idx].strip()
            else:
                # Neither, done
                break
        return base_type

    def get_is_directly_embedded(self):
        # True if not pointer
        return not self.typestr.endswith("*")

    def format_type_reference(self):
        if not self.typestr.endswith("]"):
            # Normal, not array
            return "{type} {name}".format(
                type=self.typestr, name=self.name)

        # Arrays
        open_brace_idx = self.typestr.rfind("[")
        if open_brace_idx == -1:
            print("ERROR: Mismatched ] in field of type {}"
                  .format(self.typestr))
            assert False
        arr_type_part = self.typestr[:open_brace_idx].strip()
        arr_size_part = self.typestr[open_brace_idx:]
        return "{type} {name}{arrpart}".format(
            type=arr_type_part, name=self.name, arrpart=arr_size_part)

    def __str__(self):
        return ("<Typpo Fieldref: {name} --> {type} ({typeobj})".format(
            name=self.name,
            type=self.typestr,
            typeobj=self.typpo_type))

    def __repr__(self):
        return self.__str__()


class TyppoTypeNode(object):
    def __init__(self, _name, _kind):
        # Common to all types
        self.name = _name
        self.kind = _kind
        # Used for DAG linearization
        self.temp_mark = False
        self.perm_mark = False
        # Fields in this type (used for struct/union)
        self.fields = []
        # Used for alien types
        self.no_emit = False
        # No padding between elements
        self.packed = False
        # What is represented by this data? A combination of size and type such
        # as float/int/etc. Only valid for base types
        self.size = 0
        self.represents = None

    def __str__(self):
        return ("<Typpo Type: \"{name}\" ({kind}), Flags: {tf}{pf}{ef}{kf}, "
                "fields: {fields}, c_type: ({size},{represents})>".format(
                    name=self.name,
                    kind=self.kind,
                    tf="T" if self.temp_mark else "",
                    pf="P" if self.perm_mark else "",
                    ef="E" if self.no_emit else "",
                    kf="K" if self.packed else "",
                    fields=self.fields,
                    size=self.size,
                    represents=self.represents))

    def __repr__(self):
        return self.__str__()


class TyppoParser(object):
    # Pass in the list of types loaded from the YAML file
    def __init__(self, input_objects):
        self._input_objects = input_objects

    # First pass collects all the type names. Also marks alien types and
    # packed structures.
    def do_initial_pass(self):
        self._types = {}
        for type_desc in self._input_objects:
            name = type_desc['name'].strip()
            kind = type_desc['kind'].strip()

            new_type = TyppoTypeNode(name, kind)
            if kind == "alien":
                new_type.no_emit = True
                new_type.size = type_desc['size']
            elif kind == "base":
                new_type.size = type_desc['size']
                if 'repr' in type_desc:
                    new_type.represents = type_desc['repr']
                else:
                    new_type.represents = 'unsigned'
            elif kind == "struct" or kind == "union":
                if 'packed' in type_desc:
                    new_type.packed = type_desc['packed']
                for field in type_desc['slots']:
                    field_name = field['name'].strip()
                    field_type = field['type'].strip()
                    new_type.fields.append(
                        TyppoFieldRef(field_name, field_type))
            else:
                print("WARNING: Unknown kind {}".format(kind))

            if name in self._types:
                print("WARNING: Redefinition of type {}".format(name))
            self._types[name] = new_type

    def resolve_references(self):
        for type_obj in self._types.values():
            if type_obj.kind == "alien" or type_obj.kind == "base":
                pass
            elif type_obj.kind == "struct" or type_obj.kind == "union":
                for field in type_obj.fields:
                    slot_type = field.get_base_type()
                    is_embedded = field.get_is_directly_embedded()

                    if not slot_type in self._types:
                        print("WARNING: Type {parent} references "
                              "unknown type {child}".format(
                                  parent=type_obj.name, child=slot_type))
                        # TODO(rqou): Report better errors
                        if is_embedded:
                            assert False

                    if is_embedded:
                        field.typpo_type = self._types[slot_type]

    def do_dag_linearization(self):
        # Uses second algorithm from
        # http://en.wikipedia.org/wiki/Topological_sorting
        sorted_types = []
        nodes_unmarked = set(self._types.values())

        def visit(n):
            if n.temp_mark:
                print("ERROR: Types have cyclic references to {name}".format(
                    name=n.name))
                assert False

            if not n.temp_mark and not n.perm_mark:
                n.temp_mark = True
                for m in n.fields:
                    if m.get_is_directly_embedded():
                        visit(m.typpo_type)
                n.perm_mark = True
                # TODO(rqou): right?
                n.temp_mark = False
                nodes_unmarked.remove(n)
                sorted_types.append(n)
        while len(nodes_unmarked) > 0:
            a_node = nodes_unmarked.pop()
            # TODO(rqou): Do I need to do this?
            nodes_unmarked.add(a_node)
            visit(a_node)
        self._sorted_types = sorted_types

    C_TYPE_EQUIVALENT = {
        (1, 'unsigned'):        'uint8_t',
        (1, 'signed'):          'int8_t',
        (2, 'unsigned'):        'uint16_t',
        (2, 'signed'):          'int16_t',
        (4, 'unsigned'):        'uint32_t',
        (4, 'signed'):          'int32_t',
        (8, 'unsigned'):        'uint64_t',
        (8, 'signed'):          'int64_t',
        # TODO(rqou): This is only valid for ARM (and x86) but not x86_64
        (4, 'float'):           'float',

        ('native', 'float'):    'float',
        ('native', 'unsigned'): 'unsigned int',
        ('native', 'signed'):   'signed int',
        ('native', 'ptr'):      'void *',
    }

    def emit_typedefs_pass1(self, f):
        for type_obj in self._types.values():
            if type_obj.no_emit:
                # Don't output these
                pass
            elif type_obj.kind == "base":
                type_representation = (type_obj.size, type_obj.represents)
                if not type_representation in self.C_TYPE_EQUIVALENT:
                    print("ERROR: No C equivalent for {}"
                          .format(type_representation))
                    assert False
                c_type = self.C_TYPE_EQUIVALENT[type_representation]
                f.write("typedef {ctype} {newtype};\n".format(
                    ctype=c_type, newtype=type_obj.name))
            elif type_obj.kind == "struct" or type_obj.kind == "union":
                f.write(
                    "{struct_union} {name};\n"
                    "typedef {struct_union} {name} {name};\n".format(
                        struct_union=type_obj.kind,
                        name=type_obj.name))
            else:
                print("WARNING: Don't know how to emit for kind {} (pass1)"
                      .format(kind))

        f.write("\n")

    def emit_typedefs_pass2(self, f):
        for type_obj in self._sorted_types:
            if type_obj.kind == "alien" or type_obj.kind == "base":
                # These have already been emitted
                pass
            elif type_obj.kind == "struct" or type_obj.kind == "union":
                # TODO(rqou): This is GCC specific, do we care?
                #print(type_obj)
                packed_decorator = (
                    "__attribute__((__packed__)) " if type_obj.packed else "")

                f.write("{struct_union} {packed}{name} {{\n"
                        .format(
                            struct_union=type_obj.kind,
                            name=type_obj.name,
                            packed=packed_decorator))
                for field in type_obj.fields:
                    f.write("    {typeref};\n".format(
                        typeref=field.format_type_reference()))
                f.write("};\n")
            else:
                print("WARNING: Don't know how to emit for kind {} (pass2)"
                      .format(kind))


def main():
    if len(sys.argv) < 3:
        print("Usage: %s input.yaml output.h" % sys.argv[0])
        sys.exit(1)

    input_filename = sys.argv[1]
    output_filename = sys.argv[2]

    inf = open(input_filename, 'r')
    inputYaml = inf.read()
    inf.close()
    inputObjects = yaml.load(inputYaml)

    typpo_parser = TyppoParser(inputObjects)
    typpo_parser.do_initial_pass()
    typpo_parser.resolve_references()
    typpo_parser.do_dag_linearization()

    with open(output_filename, 'w') as outf:
        typpo_emit_header_start(outf, input_filename)
        typpo_parser.emit_typedefs_pass1(outf)
        typpo_parser.emit_typedefs_pass2(outf)
        typpo_emit_header_end(outf, input_filename)


if __name__ == '__main__':
    main()
