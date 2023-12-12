import os
import re

def extract_classes_and_relations(file_content):
    # Extract class names
    classes = re.findall(r'class (\w+)', file_content)

    # Extract relationships (simplified example, adapt as needed)
    relationships = re.findall(r'(\w+) --\|> (\w+)', file_content)

    return classes, relationships

def generate_plantuml_markup(classes, relationships):
    plantuml_code = "@startuml\n"

    # Generate PlantUML code for classes
    for class_name in classes:
        plantuml_code += f"class {class_name} {{\n}}\n"

    # Generate PlantUML code for relationships
    for relationship in relationships:
        plantuml_code += f"{relationship[0]} --|> {relationship[1]}\n"

    plantuml_code += "@enduml\n"

    return plantuml_code

def process_files(root_directory):
    for foldername, subfolders, filenames in os.walk(root_directory):
        for filename in filenames:
            if filename.endswith(('.cpp', '.h', '.java', '.py', '.hpp')):  # Adjust file extensions as needed
                file_path = os.path.join(foldername, filename)
                with open(file_path, 'r') as file:
                    file_content = file.read()
                    classes, relationships = extract_classes_and_relations(file_content)
                    if classes or relationships:
                        plantuml_code = generate_plantuml_markup(classes, relationships)
                        # Modify the file or save the generated PlantUML code as needed

if __name__ == "__main__":
    root_directory = "/home/tbashir/traccc"
    process_files(root_directory)

