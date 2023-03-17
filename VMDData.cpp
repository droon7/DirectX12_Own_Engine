#include "pch.h"
#include "VMDData.h"

VMDData::VMDData(std::string strVMDPath)
	:duration(0)
{
	LoadVMDData(strVMDPath);
	SetMotionDatas();
}


//VMDデータ読み込み
void VMDData::LoadVMDData(std::string strVMDPath)
{
	FILE* fp;
	auto err = fopen_s(&fp, strVMDPath.c_str(), "rb");



	fseek(fp, 50, SEEK_SET);

	motionDataNum = 0;
	fread(&motionDataNum, sizeof(motionDataNum), 1, fp);

	vmdMotionDatas.resize(motionDataNum);
	for (auto& motion : vmdMotionDatas)
	{
		fread(motion.boneName, sizeof(motion.boneName), 1, fp);
		fread(&motion.frameNo, sizeof(motion.frameNo)
			+ sizeof(motion.location)
			+ sizeof(motion.quaternion)
			+ sizeof(motion.bezier),
			1,
			fp);

	}
	
	if (strVMDPath == std::string("motion/squat.vmd"))
	{
		return;
	}

	uint32_t morphCount = 0;
	fread(&morphCount, sizeof(morphCount), 1, fp);
	vmdMorphs.resize(morphCount);
	fread(vmdMorphs.data(), sizeof(VMDMorph), morphCount, fp);

	uint32_t cameraCount = 0;
	fread(&cameraCount, sizeof(cameraCount), 1, fp);
	vmdCameras.resize(cameraCount);
	fread(vmdCameras.data(), sizeof(VMDCamera), cameraCount, fp);

	uint32_t lightCount = 0;
	fread(&lightCount, sizeof(lightCount), 1, fp);
	vmdLights.resize(lightCount);
	fread(vmdLights.data(), sizeof(VMDLight), lightCount, fp);

	uint32_t selfShadowCount = 0;
	fread(&selfShadowCount, sizeof(selfShadowCount), 1, fp);
	vmdSelfShadows.resize(selfShadowCount);
	fread(vmdSelfShadows.data(), sizeof(VMDSelfShadow), selfShadowCount, fp);

	//IKスイッチ読み込み
	uint32_t ikSwitchCount = 0;
	fread(&ikSwitchCount, sizeof(ikSwitchCount), 1, fp);
	vmdIkEnableDatas.resize(ikSwitchCount);

	for (auto& ikEnable : vmdIkEnableDatas)
	{
		fread(&ikEnable.frameNo, sizeof(ikEnable.frameNo), 1, fp);

		//使用しない可視フラグ1バイト
		uint8_t visibleflag = 0;
		fread(&visibleflag, sizeof(visibleflag), 1, fp);

		//対象ボーン数読み込み
		uint32_t ikBoneCount = 0;
		fread(&ikBoneCount, sizeof(ikBoneCount), 1, fp);

		//ボーン名とそれのオン/オフ情報読み込み
		for (int i = 0; i < ikBoneCount; ++i)
		{
			char ikBoneName[20];
			fread(ikBoneName, sizeof(ikBoneName), 1, fp);

			uint8_t flg = 0;
			fread(&flg, sizeof(flg), 1, fp);
			ikEnable.ikEnableTable[ikBoneName] = flg;
		}
	}

	fclose(fp);
}

void VMDData::SetMotionDatas()
{

	for (auto& vmdMotion : vmdMotionDatas)
	{
		//クォータニオン、コントロールポイントを一次変数に格納
		auto q = DirectX::XMLoadFloat4(&vmdMotion.quaternion);
		auto cp1 = DirectX::XMFLOAT2(static_cast<float>(vmdMotion.bezier[3]) / 127.0f,
			static_cast<float>(vmdMotion.bezier[7]) / 127.0f );
		auto cp2 = DirectX::XMFLOAT2(static_cast<float>(vmdMotion.bezier[11]) / 127.0f,
			static_cast<float>(vmdMotion.bezier[15]) / 127.0f);

		//モーションテーブルにモーション構造体に格納
		motionDatas[vmdMotion.boneName].emplace_back(
			Motion(vmdMotion.frameNo, q,vmdMotion.location, cp1, cp2)
		);

		duration = std::max<unsigned int>(duration, vmdMotion.frameNo);
	}

	//フレーム番号でソートする
	auto isLessThanFrameNo = [](const Motion& leftval, const Motion& rightval)
	{
		return leftval.frameNo <= rightval.frameNo;
	};
	for (auto& motion : motionDatas)
	{
		std::sort(motion.second.begin(), motion.second.end(), isLessThanFrameNo);
	}

}

