#!/usr/bin/env python3
"""
Simple script to convert Doxygen XML output to Markdown documentation
for the Connection Library public API functions.
"""

import xml.etree.ElementTree as ET
import sys
from pathlib import Path


def clean_text(text):
    """Clean and format text content from XML."""
    if text is None:
        return ""
    return text.strip().replace("\n", " ").replace("  ", " ")


def extract_function_signature(member):
    """Extract function signature from member definition."""
    definition = member.find("definition")
    argsstring = member.find("argsstring")

    if definition is not None and argsstring is not None:
        def_text = definition.text or ""
        args_text = argsstring.text or ""
        return f"{def_text}{args_text}"

    return ""


def parse_enums(xml_file):
    """Parse enum documentation from XML file."""
    try:
        tree = ET.parse(xml_file)
        root = tree.getroot()

        enums = []

        # Find all enum definitions
        for member in root.findall('.//memberdef[@kind="enum"]'):
            # Get enum name
            name_elem = member.find("name")
            if name_elem is None:
                continue
            enum_name = name_elem.text

            # Get brief description
            brief = ""
            briefdesc = member.find("briefdescription")
            if briefdesc is not None:
                para = briefdesc.find("para")
                if para is not None and para.text:
                    brief = clean_text(para.text)

            # Get detailed description
            detailed = ""
            detaileddesc = member.find("detaileddescription")
            if detaileddesc is not None:
                para = detaileddesc.find("para")
                if para is not None and para.text:
                    detailed = clean_text(para.text)

            # Get enum values
            values = []
            for enumvalue in member.findall(".//enumvalue"):
                value_name_elem = enumvalue.find("name")
                if value_name_elem is None:
                    continue

                value_name = value_name_elem.text

                # Get value description
                value_desc = ""
                value_briefdesc = enumvalue.find("briefdescription")
                if value_briefdesc is not None:
                    para = value_briefdesc.find("para")
                    if para is not None and para.text:
                        value_desc = clean_text(para.text)

                # Get initializer (value)
                initializer = ""
                init_elem = enumvalue.find("initializer")
                if init_elem is not None and init_elem.text:
                    initializer = init_elem.text.strip("= ")

                values.append(
                    {
                        "name": value_name,
                        "value": initializer,
                        "description": value_desc,
                    }
                )

            # Only include enums that have documentation or documented values
            if brief or detailed or any(v["description"] for v in values):
                enums.append(
                    {
                        "name": enum_name,
                        "brief": brief,
                        "detailed": detailed,
                        "values": values,
                    }
                )

        return enums

    except ET.ParseError as e:
        print(f"Error parsing {xml_file}: {e}")
        return []


def parse_structs(xml_file):
    """Parse struct documentation from XML file."""
    try:
        tree = ET.parse(xml_file)
        root = tree.getroot()

        structs = []

        # Find all struct definitions
        for compound in root.findall('.//compounddef[@kind="struct"]'):
            # Get struct name
            name_elem = compound.find("compoundname")
            if name_elem is None:
                continue
            struct_name = name_elem.text

            # Get brief description
            brief = ""
            briefdesc = compound.find("briefdescription")
            if briefdesc is not None:
                para = briefdesc.find("para")
                if para is not None and para.text:
                    brief = clean_text(para.text)

            # Get detailed description
            detailed = ""
            detaileddesc = compound.find("detaileddescription")
            if detaileddesc is not None:
                para = detaileddesc.find("para")
                if para is not None and para.text:
                    detailed = clean_text(para.text)

            # Get member variables
            members = []
            for member in compound.findall('.//memberdef[@kind="variable"]'):
                member_name_elem = member.find("name")
                if member_name_elem is None:
                    continue

                member_name = member_name_elem.text

                # Get member type
                member_type = ""
                type_elem = member.find("type")
                if type_elem is not None:
                    if type_elem.text:
                        member_type = type_elem.text.strip()
                    for ref in type_elem.findall("ref"):
                        if ref.text:
                            member_type += ref.text
                        if ref.tail:
                            member_type += ref.tail

                # Get member description
                member_desc = ""
                member_briefdesc = member.find("briefdescription")
                if member_briefdesc is not None:
                    para = member_briefdesc.find("para")
                    if para is not None and para.text:
                        member_desc = clean_text(para.text)

                members.append(
                    {
                        "name": member_name,
                        "type": member_type.strip(),
                        "description": member_desc,
                    }
                )

            # Only include structs that have documentation or documented members
            if brief or detailed or any(m["description"] for m in members):
                structs.append(
                    {
                        "name": struct_name,
                        "brief": brief,
                        "detailed": detailed,
                        "members": members,
                    }
                )

        return structs

    except ET.ParseError as e:
        print(f"Error parsing {xml_file}: {e}")
        return []


def parse_functions(xml_file):
    """Parse function documentation from XML file."""
    try:
        tree = ET.parse(xml_file)
        root = tree.getroot()

        functions = []

        # Find all function members with documentation
        for member in root.findall('.//memberdef[@kind="function"]'):
            # Check for documentation
            briefdesc = member.find("briefdescription")
            detaileddesc = member.find("detaileddescription")

            has_docs = False
            if briefdesc is not None:
                para = briefdesc.find("para")
                if para is not None and para.text and para.text.strip():
                    has_docs = True

            if not has_docs and detaileddesc is not None:
                para = detaileddesc.find("para")
                if para is not None and para.text and para.text.strip():
                    has_docs = True

            if not has_docs:
                continue

            # Extract function information
            name_elem = member.find("name")
            func_name = name_elem.text if name_elem is not None else "Unknown"

            # Get function signature
            signature = extract_function_signature(member)

            # Get brief description
            brief = ""
            if briefdesc is not None:
                para = briefdesc.find("para")
                if para is not None and para.text:
                    brief = clean_text(para.text)

            # Get detailed description
            detailed = ""
            if detaileddesc is not None:
                para = detaileddesc.find("para")
                if para is not None and para.text:
                    detailed = clean_text(para.text)

            # Get parameter descriptions
            param_docs = {}
            for paramlist in member.findall('.//parameterlist[@kind="param"]'):
                for item in paramlist.findall("parameteritem"):
                    namelist = item.find("parameternamelist")
                    desc = item.find("parameterdescription")

                    if namelist is not None and desc is not None:
                        name_elem = namelist.find("parametername")
                        if name_elem is not None and name_elem.text:
                            desc_para = desc.find("para")
                            if desc_para is not None and desc_para.text:
                                param_docs[name_elem.text] = clean_text(desc_para.text)

            # Get return description
            return_desc = ""
            for sect in member.findall('.//simplesect[@kind="return"]'):
                para = sect.find("para")
                if para is not None and para.text:
                    return_desc = clean_text(para.text)

            functions.append(
                {
                    "name": func_name,
                    "signature": signature,
                    "brief": brief,
                    "detailed": detailed,
                    "param_docs": param_docs,
                    "return_desc": return_desc,
                }
            )

        return functions

    except ET.ParseError as e:
        print(f"Error parsing {xml_file}: {e}")
        return []


def generate_markdown(functions, structs, enums, output_file):
    """Generate Markdown documentation."""
    with open(output_file, "w", encoding="utf-8") as f:
        f.write("# Connection Library API Documentation\n\n")
        f.write(
            "This document provides documentation for the public API of the Connection Library.\n\n"
        )

        # Table of contents
        f.write("## Table of Contents\n\n")

        # Enums section
        if enums:
            f.write("### Enumerations\n\n")
            for enum in enums:
                enum_anchor = (
                    enum["name"]
                    .lower()
                    .replace("_", "-")
                    .replace("e-", "")
                    .replace("t-", "")
                )
                f.write(f"- [{enum['name']}](#{enum_anchor})\n")
            f.write("\n")

        # Structs section
        if structs:
            f.write("### Data Structures\n\n")
            for struct in structs:
                struct_anchor = (
                    struct["name"]
                    .lower()
                    .replace("_", "-")
                    .replace("s-", "")
                    .replace("t-", "")
                )
                f.write(f"- [{struct['name']}](#{struct_anchor})\n")
            f.write("\n")

        # Functions section
        if functions:
            f.write("### Functions\n\n")
            for func in functions:
                f.write(
                    f"- [{func['name']}](#{func['name'].lower().replace('_', '-')})\n"
                )
            f.write("\n")

        # Enumerations documentation
        if enums:
            f.write("## Enumerations\n\n")

            for enum in enums:
                enum_anchor = (
                    enum["name"]
                    .lower()
                    .replace("_", "-")
                    .replace("e-", "")
                    .replace("t-", "")
                )
                f.write(f"### {enum['name']}\n\n")

                # Brief description
                if enum["brief"]:
                    f.write(f"**Brief:** {enum['brief']}\n\n")

                # Detailed description
                if enum["detailed"] and enum["detailed"] != enum["brief"]:
                    f.write(f"**Description:** {enum['detailed']}\n\n")

                # Values
                if enum["values"]:
                    f.write("**Values:**\n\n")
                    f.write("| Name | Value | Description |\n")
                    f.write("|------|-------|-------------|\n")
                    for value in enum["values"]:
                        value_desc = (
                            value["description"].replace("|", "\\|")
                            if value["description"]
                            else "-"
                        )
                        value_val = value["value"] if value["value"] else "-"
                        f.write(
                            f"| `{value['name']}` | `{value_val}` | {value_desc} |\n"
                        )
                    f.write("\n")

                f.write("---\n\n")

        # Data Structures documentation
        if structs:
            f.write("## Data Structures\n\n")

            for struct in structs:
                struct_anchor = (
                    struct["name"]
                    .lower()
                    .replace("_", "-")
                    .replace("s-", "")
                    .replace("t-", "")
                )
                f.write(f"### {struct['name']}\n\n")

                # Brief description
                if struct["brief"]:
                    f.write(f"**Brief:** {struct['brief']}\n\n")

                # Detailed description
                if struct["detailed"] and struct["detailed"] != struct["brief"]:
                    f.write(f"**Description:** {struct['detailed']}\n\n")

                # Members
                if struct["members"]:
                    f.write("**Members:**\n\n")
                    f.write("| Type | Name | Description |\n")
                    f.write("|------|------|-------------|\n")
                    for member in struct["members"]:
                        member_type = member["type"].replace("|", "\\|")
                        member_desc = (
                            member["description"].replace("|", "\\|")
                            if member["description"]
                            else "-"
                        )
                        f.write(
                            f"| `{member_type}` | `{member['name']}` | {member_desc} |\n"
                        )
                    f.write("\n")

                f.write("---\n\n")

        # Function documentation
        if functions:
            f.write("## Functions\n\n")

            for func in functions:
                f.write(f"### {func['name']}\n\n")

                # Function signature
                if func["signature"]:
                    f.write(f"```c\n{func['signature']}\n```\n\n")

                # Brief description
                if func["brief"]:
                    f.write(f"**Brief:** {func['brief']}\n\n")

                # Detailed description
                if func["detailed"] and func["detailed"] != func["brief"]:
                    f.write(f"**Description:** {func['detailed']}\n\n")

                # Parameters
                if func["param_docs"]:
                    f.write("**Parameters:**\n\n")
                    for param_name, param_desc in func["param_docs"].items():
                        f.write(f"- `{param_name}`: {param_desc}\n")
                    f.write("\n")

                # Return value
                if func["return_desc"]:
                    f.write(f"**Returns:** {func['return_desc']}\n\n")

                f.write("---\n\n")


def main():
    # Define paths
    xml_dir = Path("xml")
    output_file = Path("docs.md")

    if not xml_dir.exists():
        print("Error: XML documentation directory not found.")
        print("Please run 'doxygen Doxyfile' first.")
        sys.exit(1)

    # Parse core_lib.h XML file
    core_lib_xml = xml_dir / "core__lib_8h.xml"

    if not core_lib_xml.exists():
        print(f"Error: {core_lib_xml} not found.")
        sys.exit(1)

    print(f"Parsing {core_lib_xml}...")

    # Parse enums from core_lib.h
    print("Parsing enum definitions...")
    enums = parse_enums(core_lib_xml)

    # Parse structs from the XML files
    print("Parsing struct definitions...")
    structs = []

    # Parse structs from individual struct XML files
    struct_files = list(xml_dir.glob("structs__*.xml"))
    for struct_file in struct_files:
        print(f"  Parsing {struct_file.name}...")
        file_structs = parse_structs(struct_file)
        structs.extend(file_structs)

    # Parse functions from core_lib.h
    functions = parse_functions(core_lib_xml)

    if not functions and not structs and not enums:
        print("No documented functions, structs, or enums found.")
        sys.exit(1)

    print(f"Found {len(enums)} documented enums:")
    for enum in enums:
        print(f"  - {enum['name']}")

    print(f"Found {len(structs)} documented structs:")
    for struct in structs:
        print(f"  - {struct['name']}")

    print(f"Found {len(functions)} documented functions:")
    for func in functions:
        print(f"  - {func['name']}")

    # Generate markdown
    print(f"Generating Markdown documentation: {output_file}")
    generate_markdown(functions, structs, enums, output_file)

    print("Done! Markdown documentation generated successfully.")


if __name__ == "__main__":
    main()
