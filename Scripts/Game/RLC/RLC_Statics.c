class RLC_Statics
{
	//Thanks to narcoleptic marshmallow for his message on the arma discord: https://discord.com/channels/105462288051380224/976155351999201390/978395568453865622 
	static void GetAllChildren(IEntity parent, notnull inout array<IEntity> allChildren)
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
 	
	static RLC_AISpawnerComponent GetAiFromEnitity(IEntity entity,array<IEntity> childrenTmp)
	{
		auto result =  new Tuple2<RLC_AISpawnerComponent, IEntity>( null, null );
		auto ai = RLC_AISpawnerComponent.Cast(entity.FindComponent(RLC_AISpawnerComponent));
		if(ai) 
		{
			return ai;
		}
		GetAllChildren(entity,childrenTmp);
		if(childrenTmp && childrenTmp.Count()>=0) 
		{
			foreach (int i, IEntity childTmp : childrenTmp)
			{	
				ai = RLC_AISpawnerComponent.Cast(childTmp.FindComponent(RLC_AISpawnerComponent));
				if(ai) 
				{
						return ai;
				}
				
			}
		}
		return null;
	}
	
}