#include "pch.h"
#include "PmdBone.h"

PmdBone::PmdBone()
{
}

PmdBone::PmdBone(std::vector<PmdBoneData> pmdBoneDatas)
{
	CreateBoneNodeTable(pmdBoneDatas);
	InitBoneMatrices(pmdBoneDatas);
}

//�e�q�֌W�̂���{�[���m�[�h�e�[�u�������
void PmdBone::CreateBoneNodeTable(std::vector<PmdBoneData> pmdBoneDatas)
{
	boneNames.resize(pmdBoneDatas.size());

	//�{�[���l�[���Y�ƘA�z�z��ɑS�{�[���f�[�^������
	for (int idx = 0; idx < pmdBoneDatas.size(); ++idx)
	{
		auto& pmdbonedata = pmdBoneDatas[idx];
		boneNames[idx] = pmdbonedata.boneName;
		auto& node = boneNodeTable[pmdbonedata.boneName];
		node.boneIdx = idx;
		node.startPos = pmdbonedata.pos;
	}

	//�S�Ă̐e�q�֌W�����
	for (auto& pmdbonedata : pmdBoneDatas)
	{
		//����continue
		if (pmdbonedata.parentNo >= pmdBoneDatas.size())
		{
			continue;
		}

		//�C�e���[�^���w���m�[�h�̐e�m�[�h���i�[���A������g���e�m�[�h�̎q�����o�Ɏ�����ǉ�����B
		//�e:�q�̊֌W��1:1-n�ƂȂ�A�t�͂��肦�Ȃ�
		auto parentName = boneNames[pmdbonedata.parentNo];
		boneNodeTable[parentName].children.emplace_back(&boneNodeTable[pmdbonedata.boneName]);
	}

}

//�{�[���ϊ��s�񏉊���
void PmdBone::InitBoneMatrices(std::vector<PmdBoneData> pmdBoneDatas)
{
	boneMatrices.resize(pmdBoneDatas.size());

	std::fill(boneMatrices.begin(), boneMatrices.end(), DirectX::XMMatrixIdentity());


}

//�t���[�������ăL�[�t���[���𔭌��A�O�L�[�t���[���ƕ�Ԃ���]�s������肷��B
void PmdBone::SetBoneMatrices(VMDData vmdData, unsigned int frameNo)
{
	std::fill(boneMatrices.begin(), boneMatrices.end(), DirectX::XMMatrixIdentity());

	for (auto& bonemotion : vmdData.motionDatas)
	{
		auto node = boneNodeTable[bonemotion.first];

		auto motions = bonemotion.second;
		//�^����ꂽ�t���[���ԍ��ƃ��[�V�����̃t���[���ԍ��������Ă��邩���肷��֐��I�u�W�F�N�g
		auto predicate = [frameNo](const Motion& motion) {
			return motion.frameNo <= frameNo;
		};

		//�߂��l�̃C�e���[�^�[�𓾂�A�����ɕ���ł���f�[�^��T������̂Ń��o�[�X�C�e���[�^�[���g��
		auto rit = std::find_if(
			motions.rbegin(), motions.rend(), predicate
		);

		//��v���Ȃ��Ȃ�continue
		if (rit == motions.rend())
		{
			continue;
		}

		//�O�L�[�t���[���Ǝ��L�[�t���[���ŕ�Ԃ��s��
		DirectX::XMMATRIX rotation;
		auto it = rit.base();

		if (it == motions.end())
		{
			rotation = DirectX::XMMatrixRotationQuaternion(rit->quaternion);
		}
		else 
		{
			auto t = static_cast<float>(frameNo - rit->frameNo)
				/ static_cast<float>(it->frameNo - rit->frameNo);

			rotation = DirectX::XMMatrixRotationQuaternion(DirectX::XMQuaternionSlerp(rit->quaternion, it->quaternion, t));
		}

		auto& pos = node.startPos;
		auto matrix = DirectX::XMMatrixTranslation(-pos.x, -pos.y, -pos.z)
			* rotation
			* DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
		boneMatrices[node.boneIdx] = matrix;
	}

	RecursiveMatrixMultiply(&boneNodeTable["�Z���^�["], DirectX::XMMatrixIdentity());
}


void PmdBone::RecursiveMatrixMultiply(BoneNode* node, const DirectX::XMMATRIX& mat)
{
	//�i�[����Ă���P�ʍs��ɓ����ϊ��s����|���A�f�[�^��ۑ�
	boneMatrices[node->boneIdx] *= mat;

	for (auto& childNode : node->children)
	{
		RecursiveMatrixMultiply(childNode, boneMatrices[node->boneIdx]);
	}
}
