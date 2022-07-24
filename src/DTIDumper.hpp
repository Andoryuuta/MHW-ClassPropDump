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
		void DumpRawInfo(std::string filename);
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
			"uEquipSelectBaseGUICtrl", // Denuvo packed GetProperties function.
			"uAchievementNotificationGUICtrl",
			"uActionHelpGUICtrl",
			"uAdditionalContentsNotificationGUICtrl",
			"uAlertAutoSaveGUICtrl",
			"uAlertVersionErrorGUICtrl",
			"uAutoSaveGUICtrl",
			"uBasicMenuTopGUICtrl",
			"uBattleAerialShowdownGUICtrl",
			"uBattleAttentionGUICtrl",
			"uBattleBoardGUICtrl",
			"uBattleBreathShowdownGUICtrl",
			"uBattleCommandSelectGUICtrl",
			"uBattleCommonSituationGUICtrl",
			"uBattleComparedPowerGUICtrl",
			"uBattleConfirmGUICtrl",
			"uBattleDoubleActionGUICtrl",
			"uBattleGUICtrl",
			"uBattleGUICtrlBase",
			"uBattleIntrusionGUICtrl",
			"uBattleItemGUICtrl",
			"uBattleKizunaAttackGUICtrl",
			"uBattleLifePointGUICtrl",
			"uBattleResultGUICtrl",
			"uBattleShowDownBaseGUICtrl",
			"uBattleShowPlayerNameGUICtrl",
			"uBattleSkillGUICtrl",
			"uBattleStraightGameGUICtrl",
			"uBattleTargetSelectGUICtrl",
			"uBattleVSBackGUICtrl",
			"uBattleVSGUICtrl",
			"uBgFilterGUICtrl",
			"uBgTextureGUICtrl",
			"uCameraOptionGUICtrl",
			"uCampConfirmRiderCardGUICtrl",
			"uCampEggConfirmGUICtrl",
			"uCampEquipPresetSelectGUICtrl",
			"uCampEquipSelectGUICtrl",
			"uCampEquipSlotSelectGUICtrl",
			"uCampItemMixGUICtrl",
			"uCampLocalMapGUICtrl",
			"uCampMenuTopGUICtrl",
			"uCampNekoTaxiGUICtrl",
			"uCampOtomonSelectGUICtrl",
			"uCampPictureBookGeneGUICtrl",
			"uCampPictureBookMonsterGUICtrl",
			"uCampPictureBookOtomonGUICtrl",
			"uCampPictureBookTopGUICtrl",
			"uCampRiderCardTopGUICtrl",
			"uCampRiderNoteGUICtrl",
			"uCampSelectRiderCardGUICtrl",
			"uCampStatusGUICtrl",
			"uCampSummaryGUICtrl",
			"uDLCStoreGUICtrl",
			"uDemoMessageGUICtrl",
			"uDemoSkipGUICtrl",
			"uDemoStaffRollMessageGUICtrl",
			"uDialogAmiiboGUICtrl",
			"uDialogBaseGUICtrl",
			"uDialogBingoDetailGUICtrl",
			"uDialogDLCContentsGUICtrl",
			"uDialogDLCItemGUICtrl",
			"uDialogDeliveryGUICtrl",
			"uDialogFortuneResultGUICtrl",
			"uDialogGeneBoardGUICtrl",
			"uDialogInputGUICtrl",
			"uDialogItemSetGUICtrl",
			"uDialogLevelUnlockGUICtrl",
			"uDialogLoadGUICtrl",
			"uDialogMaterialGUICtrl",
			"uDialogMixNumGUICtrl",
			"uDialogMultiCondBattleGUICtrl",
			"uDialogMultiCondGUICtrlBase",
			"uDialogMultiCondItemSetGUICtrl",
			"uDialogMultiCondQuestGUICtrl",
			"uDialogMultiCondTagGUICtrl",
			"uDialogMultiPrepareGUICtrl",
			"uDialogNetworkErrorGUICtrl",
			"uDialogNumGUICtrl",
			"uDialogRiderCardRecvGUICtrl",
			"uDialogRiderCardSwapGUICtrl",
			"uDialogSkillDetailGUICtrl",
			"uDialogStampGUICtrl",
			"uDialogTUNotificationGUICtrl",
			"uDialogTicketGUICtrl",
			"uDialogTrialInfoGUICtrl",
			"uDialogWindowGUICtrl",
			"uDownloadNoGUICtrl",
			"uEquipCompareGUICtrl",
			"uEquipSelectBaseGUICtrl",
			"uEquipShopGUICtrl",
			"uEquipSkillListGUICtrl",
			"uEventOptionGUICtrl",
			"uExpeditionPartyOrganizeGUICtrl",
			"uExpeditionResultGUICtrl",
			"uFieldGUICtrlBase",
			"uFieldHudGUICtrl",
			"uFieldNameDisplayGUICtrl",
			"uFieldNameDisplayInDemoGUICtrl",
			"uGUICtrlBase",
			"uGalleryTopGUICtrl",
			"uGeneTraditionGUICtrl",
			"uGeneTraditionItemGUICtrl",
			"uGeneTraditionTutorialGUICtrl",
			"uHatchEggGUICtrl",
			"uHitMarkGUICtrl",
			"uINetSelectGUICtrl",
			"uItemSelectGUICtrl",
			"uItemSetCountGUICtrl",
			"uItemSetGUICtrl",
			"uKeyConfigOptionGUICtrl",
			"uKizunaAttackGalleryDoubleGUICtrl",
			"uKizunaAttackGalleryGUICtrlBase",
			"uKizunaAttackGallerySingleGUICtrl",
			"uLanSelectGUICtrl",
			"uLanguageOptionGUICtrl",
			"uLeadToShopGUICtrl",
			"uLoadingGUICtrl",
			"uLocalSelectGUICtrl",
			"uMelynxEquipShopBaseGUICtrl",
			"uMelynxShopAccessoryGUICtrl",
			"uMelynxShopArmorGUICtrl",
			"uMelynxShopGUICtrl",
			"uMelynxShopWeaponGUICtrl",
			"uMenuCommonGUICtrl",
			"uMenuSelectGUICtrlBase",
			"uModelFrameGUICtrl",
			"uMultiEquipSlotSelectGUICtrl",
			"uMultiPartyOrganizeGUICtrl",
			"uMultiPartyPresetManageGUICtrl",
			"uMultiPrepareGUICtrl",
			"uMultiQuestFailedGUICtrl",
			"uMultiQuestResultGUICtrl",
			"uMultiQuestResultGUICtrlBase",
			"uMultiQuestVersusGUICtrl",
			"uMultiTrialQuestResultGUICtrl",
			"uMultiTrialQuestResultGUICtrlBase",
			"uMultiplayGUICtrl",
			"uMyHouseEditGUICtrlBase",
			"uMyHouseEditMenuSelectGUICtrl",
			"uMyHouseSelectMenuGUICtrlBase",
			"uMyHouseTopGUICtrl",
			"uNavigateIconGUICtrl",
			"uNavirouCoordinationGUICtrl",
			"uNetSelectGUICtrlBase",
			"uNpcBattleResultGUICtrl",
			"uNpcLayeredArmorEditGUICtrl",
			"uNpcPopupTalkGUICtrl",
			"uNpcTalkIconGUICtrl",
			"uOneLineInfoGUICtrl",
			"uOnlyButtonOptionGUICtrlBase",
			"uOnlyCheckBoxOptionGUICtrlBase",
			"uOptionAgreementSelectGUICtrl",
			"uOptionDataCollectExplainGUICtrl",
			"uOptionSettingGUICtrlBase",
			"uOptionTopGUICtrl",
			"uOthersOptionGUICtrl",
			"uOtomonExpeditionGUICtrl",
			"uOtomonSelectBaseGUICtrl",
			"uOtomonSelectGUICtrl",
			"uOtomonStatusCheckGUICtrl",
			"uPartyOrganizeGUICtrl",
			"uPartyOrganizeGUICtrlBase",
			"uPartyPresetManageGUICtrl",
			"uPrayerPotGUICtrl",
			"uPrivacyOptionGUICtrl",
			"uQuestBoardGUICtrl",
			"uQuestClearGUICtrl",
			"uQuestConfirmGUICtrl",
			"uQuestResultGUICtrl",
			"uResultBaseGUICtrl",
			"uRideActionOptionGUICtrl",
			"uRiderCardOptionGUICtrl",
			"uRiderEditGUICtrl",
			"uScreenOptionGUICtrl",
			"uSelectSaveDataGUICtrl",
			"uSkillListGUICtrl",
			"uSkillSelectGUICtrl",
			"uSoundOptionGUICtrl",
			"uStampChatGUICtrl",
			"uStampInputGUICtrl",
			"uStartUpLogoGUICtrl",
			"uStoryGalleryGUICtrl",
			"uStoryMessageGUICtrl",
			"uTeamContinueGUICtrl",
			"uTeamSelectGUICtrl",
			"uTitleMenuGUICtrl",
			"uTornamentBoardGUICtrl",
			"uTournamentResultGUICtrl",
			"uTournamentStartConfirmGUICtrl",
			"uTrialBoardGUICtrl",
			"uTrialQuestResultGUICtrl",
			"uTrialTurnCountGUICtrl",
			"uTsukinoFortuneGUICtrl",
			"uTutorialArrowGUICtrl",
			"uVarietyShopGUICtrl",
			"uWeaponSelectGUICtrl",
			"uWipeOutGUICtrl",



			// Blocking:
			//"sKeyboard", "sOtomo", "nDraw::ExternalRegion", "nDraw::IndexBuffer", "uGuideInsect",

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


