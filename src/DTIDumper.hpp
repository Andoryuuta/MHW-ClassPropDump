#pragma once
#include <cstdint>
#include <string>
#include <map>
#include <set>
#include <vector>
#include "Mt.hpp"

namespace DTIDumper {
	struct ClassRecord {
		Mt::MtDTI* dti;
		void* obj_instance;
		void* class_vftable;
		std::vector<Mt::MtProperty*> properties;
	};

	class DTIDumper
	{
	public:
		DTIDumper();
		~DTIDumper();

		void ParseDTI();
		void DumpToFile(std::string filename);
		void DumpPythonArrayFile(std::string filename);
		void DumpResourceInformation(std::string filename);

		std::vector<ClassRecord> GetClassRecords();
		std::map<std::string, Mt::MtDTI*> GetFlattenedDtiMap();
		std::vector<Mt::MtDTI*> GetSortedDtiVector();

	private:
		uint64_t image_base;
		Mt::MtDTIHashTable* mt_dti_hash_table;
		std::set<std::string> excluded_classes{
			// Blocking:
			"sKeyboard", "sOtomo", "nDraw::ExternalRegion", "nDraw::IndexBuffer", "uGuideInsect",

			/*
			// "OK"-able prompts with SEH:
			"nDraw::Scene", "nDraw::VertexBuffer", "nDraw::ExternalRegion",
			"sMhKeyboard","sMhMouse",
			"sMhMain",  "sMhRender",
			"sMouse", "sRender",
			"sMhScene",
			*/
		};
		std::vector<ClassRecord> class_records;
	};
}


