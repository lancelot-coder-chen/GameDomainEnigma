﻿#include "AnimatedPawn.h"
#include "Animators/AnimatorCommands.h"
#include "Frameworks/CommandBus.h"
#include "Animators/ModelPrimitiveAnimator.h"
#include "AvatarRecipes.h"

using namespace Enigma::GameCommon;
using namespace Enigma::SceneGraph;
using namespace Enigma::Animators;
using namespace Enigma::Engine;

DEFINE_RTTI(GameCommon, AnimatedPawn, Pawn);

AnimatedPawn::AnimatedPawn(const std::string& name) : Pawn(name)
{

}

AnimatedPawn::~AnimatedPawn()
{
    m_avatarRecipeList.clear();
}

void AnimatedPawn::PlayAnimation(const std::string& name)
{
    if (m_animationClipMap.GetAnimationClipMap().empty()) return;
    if (!m_primitive) return;

    auto action_clip = m_animationClipMap.FindAnimationClip(name);
    if (!action_clip) return;

    std::list<std::shared_ptr<Animator>> anim_list;
    EnumAnimatorListDeep(anim_list);
    if (anim_list.size() == 0) return;
    for (auto& anim : anim_list)
    {
        if (anim->TypeInfo().IsExactly(ModelPrimitiveAnimator::TYPE_RTTI))
        {
            Frameworks::CommandBus::Post(std::make_shared<AddListeningAnimator>(anim));
            if (ModelPrimitiveAnimatorPtr model_ani = std::dynamic_pointer_cast<ModelPrimitiveAnimator, Animator>(anim))
            {
                model_ani->FadeInAnimation(0.3f, action_clip->m_animClip);
            }
        }
    }
}

void AnimatedPawn::StopAnimation()
{
    if (!m_primitive) return;

    std::list<std::shared_ptr<Animator>> anim_list;
    EnumAnimatorListDeep(anim_list);
    if (anim_list.size() == 0) return;
    for (auto& anim : anim_list)
    {
        if (anim->TypeInfo().IsExactly(ModelPrimitiveAnimator::TYPE_RTTI))
        {
            Frameworks::CommandBus::Post(std::make_shared<RemoveListeningAnimator>(anim));
            if (ModelPrimitiveAnimatorPtr model_ani = std::dynamic_pointer_cast<ModelPrimitiveAnimator, Animator>(anim))
            {
                model_ani->StopAnimation();
            }
        }
    }
}

void AnimatedPawn::AddAvatarRecipe(const std::shared_ptr<AvatarRecipe>& recipe)
{
    m_avatarRecipeList.push_back(recipe);
}

void AnimatedPawn::BakeAvatarRecipes()
{
    for (auto& recipe : m_avatarRecipeList)
    {
        recipe->Bake(std::dynamic_pointer_cast<Pawn, Spatial>(shared_from_this()));
    }
}