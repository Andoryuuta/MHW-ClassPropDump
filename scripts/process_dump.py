import sys
import dump_processor

def print_available_game_profiles():
    print("Available profiles:")
    for shortname, profile in dump_processor.mt_game_profiles.items():
        print(f"\t{shortname} - {profile['game_name']}")


def main():
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <game profile> <path to dti_map_with_props.json>")
        print_available_game_profiles()
        sys.exit(1)

    profile_name = sys.argv[1].lower()
    dump_path = sys.argv[2]

    if not profile_name in dump_processor.mt_game_profiles:
        print(f"Invalid game profile: {profile_name}\n")
        print_available_game_profiles()
        sys.exit(1)

    profile = dump_processor.mt_game_profiles[profile_name]

    dp = dump_processor.DumpProcessor()
    dp.load_profile(profile)
    dp.load(dump_path)

    dp.write_flat_dump('./flat_dump.h')
    print("Wrote ./flat_dump.h")


if __name__ == '__main__':
    main()