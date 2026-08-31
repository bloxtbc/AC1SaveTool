import xml.etree.ElementTree as ET
import json
import sys
from pathlib import Path


SCRIPT_DIR = Path(__file__).resolve().parent
DATABASE_FILE = SCRIPT_DIR / "hashes.json"


def load_database():
    if not DATABASE_FILE.exists():
        return {}

    try:
        with DATABASE_FILE.open("r", encoding="utf-8") as f:
            data = json.load(f)
    except json.JSONDecodeError:
        print(f"ERROR: Invalid JSON database: {DATABASE_FILE}")
        sys.exit(1)

    # Convert JSON keys back to integers internally
    return {
        int(hash_value): list(names)
        for hash_value, names in data.items()
    }


def save_database(database):
    # JSON requires object keys to be strings
    output = {
        str(hash_value): sorted(names)
        for hash_value, names in sorted(database.items())
    }

    with DATABASE_FILE.open("w", encoding="utf-8") as f:
        json.dump(
            output,
            f,
            indent=1,
            ensure_ascii=False
        )


def extract_hashes(xml_file):
    print(f"Reading: {xml_file}")

    tree = ET.parse(xml_file)
    root = tree.getroot()

    hashes = {}

    for element in root.iter("Hash32"):
        name = element.attrib.get("HashValue")
        text = element.text

        if not name or text is None:
            continue

        text = text.strip()

        try:
            value = int(text, 0)
        except ValueError:
            print(
                f"WARNING: Could not parse hash "
                f"{text!r} in {xml_file}"
            )
            continue

        hashes.setdefault(value, [])

        if name not in hashes[value]:
            hashes[value].append(name)

    return hashes


def merge_hashes(database, extracted):
    new_hashes = 0
    new_names = 0

    for hash_value, names in extracted.items():

        if hash_value not in database:
            database[hash_value] = []
            new_hashes += 1

        for name in names:
            if name not in database[hash_value]:
                database[hash_value].append(name)
                new_names += 1

    return new_hashes, new_names


def main():
    if len(sys.argv) < 2:
        print(
            "Usage:\n"
            "  python hash.py <xml1> [xml2] [xml3] ...\n\n"
            "Examples:\n"
            "  python hash.py export.xml\n"
            "  python hash.py save1.xml save2.xml save3.xml\n"
            "  python hash.py *.xml"
        )
        return 1

    database = load_database()

    print(f"Existing database entries: {len(database)}")
    print()

    total_new_hashes = 0
    total_new_names = 0

    for filename in sys.argv[1:]:
        xml_file = Path(filename)

        if not xml_file.exists():
            print(f"WARNING: File not found: {xml_file}")
            continue

        try:
            extracted = extract_hashes(xml_file)
        except ET.ParseError as e:
            print(f"ERROR: Invalid XML: {xml_file}")
            print(f"       {e}")
            continue

        new_hashes, new_names = merge_hashes(
            database,
            extracted
        )

        total_new_hashes += new_hashes
        total_new_names += new_names

        print(
            f"  Found {len(extracted)} unique hashes"
        )
        print(
            f"  New hashes: {new_hashes}"
        )
        print(
            f"  New names:  {new_names}"
        )
        print()

    save_database(database)

    print("========== SUMMARY ==========")
    print(f"Database hashes: {len(database)}")
    print(f"New hashes:      {total_new_hashes}")
    print(f"New names:       {total_new_names}")
    print(f"Database:        {DATABASE_FILE}")

    return 0


if __name__ == "__main__":
    sys.exit(main())