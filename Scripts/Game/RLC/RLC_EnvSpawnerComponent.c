[ComponentEditorProps(category: "GameScripted/Coop", description: "Allows spawning of AI groups.")]
class SCR_EnvSpawnerComponentClass : ScriptComponentClass
{
}

void ScriptInvoker_OnSpawnerEmptyDelegateEnv();
typedef func ScriptInvoker_OnSpawnerEmptyDelegateEnv;
typedef ScriptInvokerBase<ScriptInvoker_OnSpawnerEmptyDelegateEnv> ScriptInvoker_OnSpawnerEmptyEnv;


//------------------------------------------------------------------------------------------------
class SCR_EnvSpawnerComponent : ScriptComponent
{
	[Attribute("{07FBB9A2F51332E4}Prefabs/Rocks/Granite/Granite_BeachStone_01.et", UIWidgets.EditBox, "Enviroment prefab to spawn.")]
	protected ResourceName m_rnDefaultPrefab;

	[Attribute("0", UIWidgets.CheckBox, "If checked, spawns immediately.")]
	protected bool m_bSpawnImmediately;


	vector parentVector[4]
	// Attached component.
	protected RplComponent m_pRplComponent;

	//! Spawned enviroment relevant to the authority only.
	protected IEntity m_pSpawnedEnv;
	
	//! Invoker which we can hook onto - see typedef above
	protected ref ScriptInvoker_OnSpawnerEmptyEnv m_pOnEmptyInvoker = new ScriptInvoker_OnSpawnerEmptyEnv();
	
	
	IEntity GetSpawnedenviroment()
	{
		return m_pSpawnedEnv;
	}	
	
	void RemoveSpawned()
	{
		RplComponent.DeleteRplEntity(m_pSpawnedEnv, false);
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker_OnSpawnerEmptyEnv GetOnEmptyInvoker()
	{
		return m_pOnEmptyInvoker;
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	bool DoSpawn(ResourceName IEntityPrefab)
	{
		if (IsSpawned())
		{
			Print("SCR_EnvSpawnerComponent cannot spawn group; group was spawned already!");
			return false;
		}

		if (!VerifyRplComponentPresent())
		{
			Print("SCR_EnvSpawnerComponent cannot spawn group, spawner has no RplComponent!");
			return false;
		}

		if (!m_pRplComponent.IsMaster())
		{
			Print("SCR_EnvSpawnerComponent caught non-master request to spawn!");
			return false;
		}

		Resource enviromentPrefab = Resource.Load(IEntityPrefab);
		if (!enviromentPrefab)
		{
			Print(string.Format("SCR_EnvSpawnerComponent could not load '%1'", enviromentPrefab));
			return false;
		}

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;	
		spawnParams.Transform = parentVector;	
		

		BaseWorld tmp = GetOwner().GetWorld();
				
		IEntity spawnedEntity = GetGame().SpawnEntityPrefab(enviromentPrefab, tmp, spawnParams);
		if (!spawnedEntity)
		{
			Print(string.Format("SCR_EnvSpawnerComponent could not spawn '%1'", enviromentPrefab));
			return false;
		}

		IEntity enviroment = IEntity.Cast(spawnedEntity);
		if (!enviroment)
		{
			Print(string.Format("SCR_EnvSpawnerComponent spawned entity '%1' that is not of IEntity type, deleting!", enviromentPrefab));
			RplComponent.DeleteRplEntity(spawnedEntity, false);
			return false;
		}

		// Store enviroment
		m_pSpawnedEnv = enviroment;

		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected event void OnEmpty()
	{
		m_pOnEmptyInvoker.Invoke();
		RemoveSpawned();
		DoSpawnDefault();
		
	}

	//------------------------------------------------------------------------------------------------
	bool IsSpawned()
	{
		return m_pSpawnedEnv != null;
	}

	//------------------------------------------------------------------------------------------------
	bool DoSpawnDefault()
	{
		return DoSpawn(m_rnDefaultPrefab);
	}
	
#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	//protected override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
	//{
	//	super._WB_AfterWorldUpdate(owner, timeSlice);
	//	
	//	vector spawnPosition = GetSpawnPosition();
	//	Shape shape = Shape.CreateSphere(COLOR_YELLOW, ShapeFlags.ONCE | ShapeFlags.NOOUTLINE, spawnPosition, 0.3);
	//	Shape arrow = Shape.CreateArrow(GetOwner().GetOrigin(), spawnPosition, 0.1, COLOR_YELLOW, ShapeFlags.ONCE);
	//}
#endif

	//------------------------------------------------------------------------------------------------
	protected bool VerifyRplComponentPresent()
	{
		if (!m_pRplComponent)
		{
			Print("SCR_EnvSpawnerComponent does not have a RplComponent attached!");
			return false;
		}

		return true;
	}
	
	//Todo not copy past that function make static xD
	//Thanks to narcoleptic marshmallow for his message on the arma discord: https://discord.com/channels/105462288051380224/976155351999201390/978395568453865622 
	private void GetAllChildren(IEntity parent, notnull inout array<IEntity> allChildren)
    {
        if (!parent)
            return;
        
        IEntity child = parent.GetChildren();
        
        while (child)
        {
            allChildren.Insert(child);
            child = child.GetSibling();
        }
    }

	//------------------------------------------------------------------------------------------------
	protected void OnTriggerActivate()
	{

		// Spawn when whatever enters this trigger
		if (!IsSpawned())
		{
			if (DoSpawnDefault())
			{
				// Once the group is spawned, in this case let's just disable the
				// trigger, making it a complete one-shot
				GenericEntity.Cast(GetOwner()).Deactivate();
				// Additionally we could just RplComponent.DeleteEntity(GetOwner(), false);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		m_pRplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!VerifyRplComponentPresent())
			return;

		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		owner.GetTransform(parentVector);
		
		if (m_bSpawnImmediately)
		{
			// Spawning of Replicated items must not happen in EOnInit,
			// we delay the call to happen asap (after EOnInit)
			GetGame().GetCallqueue().CallLater(DoSpawnDefault, 0);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{		
		SCR_AIGroup aiGroup = SCR_AIGroup.Cast(GetSpawnedenviroment());
		if (aiGroup)
		{
			aiGroup.GetOnEmpty().Remove(OnEmpty);
		}
	}

	//------------------------------------------------------------------------------------------------
	void SCR_EnvSpawnerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_EnvSpawnerComponent()
	{
	}

}
