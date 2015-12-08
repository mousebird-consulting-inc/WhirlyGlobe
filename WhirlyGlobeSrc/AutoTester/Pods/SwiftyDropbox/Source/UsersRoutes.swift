/// Routes for the users namespace
public class UsersRoutes {
    public let client : BabelClient
    init(client: BabelClient) {
        self.client = client
    }
    /**
        Get information about a user's account.

        - parameter accountId: A user's account identifier.

         - returns: Through the response callback, the caller will receive a `Users.BasicAccount` object on success or a
        `Users.GetAccountError` object on failure.
    */
    public func getAccount(accountId accountId: String) -> BabelRpcRequest<Users.BasicAccountSerializer, Users.GetAccountErrorSerializer> {
        let request = Users.GetAccountArg(accountId: accountId)
        return BabelRpcRequest(client: self.client, host: "meta", route: "/users/get_account", params: Users.GetAccountArgSerializer().serialize(request), responseSerializer: Users.BasicAccountSerializer(), errorSerializer: Users.GetAccountErrorSerializer())
    }
    /**
        Get information about the current user's account.


         - returns: Through the response callback, the caller will receive a `Users.FullAccount` object on success or a
        `Void` object on failure.
    */
    public func getCurrentAccount() -> BabelRpcRequest<Users.FullAccountSerializer, VoidSerializer> {
        return BabelRpcRequest(client: self.client, host: "meta", route: "/users/get_current_account", params: Serialization._VoidSerializer.serialize(), responseSerializer: Users.FullAccountSerializer(), errorSerializer: Serialization._VoidSerializer)
    }
    /**
        Get the space usage information for the current user's account.


         - returns: Through the response callback, the caller will receive a `Users.SpaceUsage` object on success or a
        `Void` object on failure.
    */
    public func getSpaceUsage() -> BabelRpcRequest<Users.SpaceUsageSerializer, VoidSerializer> {
        return BabelRpcRequest(client: self.client, host: "meta", route: "/users/get_space_usage", params: Serialization._VoidSerializer.serialize(), responseSerializer: Users.SpaceUsageSerializer(), errorSerializer: Serialization._VoidSerializer)
    }
    /**
        Get information about multiple user accounts.  At most 300 accounts may be queried per request.

        - parameter accountIds: List of user account identifiers.  Should not contain any duplicate account IDs.

         - returns: Through the response callback, the caller will receive a `Array<Users.BasicAccount>` object on
        success or a `Users.GetAccountBatchError` object on failure.
    */
    public func getAccountBatch(accountIds accountIds: Array<String>) -> BabelRpcRequest<ArraySerializer<Users.BasicAccountSerializer>, Users.GetAccountBatchErrorSerializer> {
        let request = Users.GetAccountBatchArg(accountIds: accountIds)
        return BabelRpcRequest(client: self.client, host: "meta", route: "/users/get_account_batch", params: Users.GetAccountBatchArgSerializer().serialize(request), responseSerializer: ArraySerializer(Users.BasicAccountSerializer()), errorSerializer: Users.GetAccountBatchErrorSerializer())
    }
}
